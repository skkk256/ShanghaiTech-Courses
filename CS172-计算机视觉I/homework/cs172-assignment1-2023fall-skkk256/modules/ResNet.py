import os
import torch
import torch.nn as nn

from torchvision import models
from torch.nn import functional as F

__all__ = [
    "ResNet",
    "resnet18",
    "resnet34",
    "resnet50",
]


def conv3x3(in_planes, out_planes, stride=1, groups=1, dilation=1):
    """3x3 convolution with padding"""
    return nn.Conv2d(
        in_planes,
        out_planes,
        kernel_size=3,
        stride=stride,
        padding=dilation,
        groups=groups,
        bias=False,
        dilation=dilation,
    )


def conv1x1(in_planes, out_planes, stride=1):
    """1x1 convolution"""
    return nn.Conv2d(in_planes, out_planes, kernel_size=1, stride=stride, bias=False)


class BasicBlock(nn.Module):
    expansion = 1

    def __init__(
        self,
        inplanes,
        planes,
        stride=1,
        downsample=None,
        groups=1,
        base_width=64,
        dilation=1,
        norm_layer=None,
    ):
        super(BasicBlock, self).__init__()
        if norm_layer is None:
            norm_layer = nn.BatchNorm2d
        if groups != 1 or base_width != 64:
            raise ValueError(
                "BasicBlock only supports groups=1 and base_width=64")
        if dilation > 1:
            raise NotImplementedError(
                "Dilation > 1 not supported in BasicBlock")
        # Both self.conv1 and self.downsample layers downsample the input when stride != 1
        self.conv1 = conv3x3(inplanes, planes, stride)
        self.bn1 = norm_layer(planes)
        self.relu = nn.ReLU(inplace=True)
        self.conv2 = conv3x3(planes, planes)
        self.bn2 = norm_layer(planes)
        self.downsample = downsample
        self.stride = stride

    def forward(self, x):
        identity = x

        out = self.conv1(x)
        out = self.bn1(out)
        out = self.relu(out)

        out = self.conv2(out)
        out = self.bn2(out)

        if self.downsample is not None:
            identity = self.downsample(x)

        out += identity
        out = self.relu(out)

        return out


class Bottleneck(nn.Module):
    expansion = 4

    def __init__(
        self,
        inplanes,
        planes,
        stride=1,
        downsample=None,
        groups=1,
        base_width=64,
        dilation=1,
        norm_layer=None,
    ):
        super(Bottleneck, self).__init__()
        if norm_layer is None:
            norm_layer = nn.BatchNorm2d
        width = int(planes * (base_width / 64.0)) * groups
        # Both self.conv2 and self.downsample layers downsample the input when stride != 1
        self.conv1 = conv1x1(inplanes, width)
        self.bn1 = norm_layer(width)
        self.conv2 = conv3x3(width, width, stride, groups, dilation)
        self.bn2 = norm_layer(width)
        self.conv3 = conv1x1(width, planes * self.expansion)
        self.bn3 = norm_layer(planes * self.expansion)
        self.relu = nn.ReLU(inplace=True)
        self.downsample = downsample
        self.stride = stride

    def forward(self, x):
        identity = x

        out = self.conv1(x)
        out = self.bn1(out)
        out = self.relu(out)

        out = self.conv2(out)
        out = self.bn2(out)
        out = self.relu(out)

        out = self.conv3(out)
        out = self.bn3(out)

        if self.downsample is not None:
            identity = self.downsample(x)

        out += identity
        out = self.relu(out)

        return out


class ResNet(nn.Module):
    def __init__(
        self,
        block,
        layers,
        num_classes=10,
        zero_init_residual=False,
        groups=1,
        width_per_group=64,
        replace_stride_with_dilation=None,
        norm_layer=None,
    ):
        super(ResNet, self).__init__()
        if norm_layer is None:
            norm_layer = nn.BatchNorm2d
        self._norm_layer = norm_layer

        self.inplanes = 64
        self.dilation = 1
        if replace_stride_with_dilation is None:
            # each element in the tuple indicates if we should replace
            # the 2x2 stride with a dilated convolution instead
            replace_stride_with_dilation = [False, False, False]
        if len(replace_stride_with_dilation) != 3:
            raise ValueError(
                "replace_stride_with_dilation should be None "
                "or a 3-element tuple, got {}".format(
                    replace_stride_with_dilation)
            )
        self.groups = groups
        self.base_width = width_per_group

        # CIFAR10: kernel_size 7 -> 3, stride 2 -> 1, padding 3->1
        self.conv1 = nn.Conv2d(
            3, self.inplanes, kernel_size=3, stride=1, padding=1, bias=False
        )
        # END

        self.bn1 = norm_layer(self.inplanes)
        self.relu = nn.ReLU(inplace=True)
        self.maxpool = nn.MaxPool2d(kernel_size=3, stride=2, padding=1)
        self.layer1 = self._make_layer(block, 64, layers[0])
        self.layer2 = self._make_layer(
            block, 128, layers[1], stride=2, dilate=replace_stride_with_dilation[0]
        )
        self.layer3 = self._make_layer(
            block, 256, layers[2], stride=2, dilate=replace_stride_with_dilation[1]
        )
        self.layer4 = self._make_layer(
            block, 512, layers[3], stride=2, dilate=replace_stride_with_dilation[2]
        )
        self.avgpool = nn.AdaptiveAvgPool2d((1, 1))
        self.fc = nn.Linear(512 * block.expansion, num_classes)

        for m in self.modules():
            if isinstance(m, nn.Conv2d):
                nn.init.kaiming_normal_(
                    m.weight, mode="fan_out", nonlinearity="relu")
            elif isinstance(m, (nn.BatchNorm2d, nn.GroupNorm)):
                nn.init.constant_(m.weight, 1)
                nn.init.constant_(m.bias, 0)

        # Zero-initialize the last BN in each residual branch,
        # so that the residual branch starts with zeros, and each residual block behaves like an identity.
        # This improves the model by 0.2~0.3% according to https://arxiv.org/abs/1706.02677
        if zero_init_residual:
            for m in self.modules():
                if isinstance(m, Bottleneck):
                    nn.init.constant_(m.bn3.weight, 0)
                elif isinstance(m, BasicBlock):
                    nn.init.constant_(m.bn2.weight, 0)

    def _make_layer(self, block, planes, blocks, stride=1, dilate=False):
        norm_layer = self._norm_layer
        downsample = None
        previous_dilation = self.dilation
        if dilate:
            self.dilation *= stride
            stride = 1
        if stride != 1 or self.inplanes != planes * block.expansion:
            downsample = nn.Sequential(
                conv1x1(self.inplanes, planes * block.expansion, stride),
                norm_layer(planes * block.expansion),
            )

        layers = []
        layers.append(
            block(
                self.inplanes,
                planes,
                stride,
                downsample,
                self.groups,
                self.base_width,
                previous_dilation,
                norm_layer,
            )
        )
        self.inplanes = planes * block.expansion
        for _ in range(1, blocks):
            layers.append(
                block(
                    self.inplanes,
                    planes,
                    groups=self.groups,
                    base_width=self.base_width,
                    dilation=self.dilation,
                    norm_layer=norm_layer,
                )
            )

        return nn.Sequential(*layers)

    def forward(self, x):
        x = self.conv1(x)
        x = self.bn1(x)
        x = self.relu(x)
        x = self.maxpool(x)

        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)
        x = self.layer4(x)

        x = self.avgpool(x)
        x = x.reshape(x.size(0), -1)
        x = self.fc(x)

        return x


def _resnet(arch, block, layers, pretrained, progress, device, checkpoints_path="", **kwargs):
    model = ResNet(block, layers, **kwargs)
    if pretrained:
        if checkpoints_path:
            checkpoints = checkpoints_path
        else:
            script_dir = os.path.dirname(__file__)
            checkpoints = script_dir + "/checkpoints/" + arch + ".pt"
        state_dict = torch.load(
            checkpoints, map_location=device
        )
        model.load_state_dict(state_dict)
    return model


def resnet18(pretrained=False, progress=True, device="cpu", **kwargs):
    """Constructs a ResNet-18 model.
    Args:
        pretrained (bool): If True, returns a model pre-trained on ImageNet
        progress (bool): If True, displays a progress bar of the download to stderr
    """
    return _resnet(
        "resnet18", BasicBlock, [2, 2, 2, 2], pretrained, progress, device, **kwargs
    )


def resnet34(pretrained=False, progress=True, device="cpu", **kwargs):
    """Constructs a ResNet-34 model.
    Args:
        pretrained (bool): If True, returns a model pre-trained on ImageNet
        progress (bool): If True, displays a progress bar of the download to stderr
    """
    return _resnet(
        "resnet34", BasicBlock, [3, 4, 6, 3], pretrained, progress, device, **kwargs
    )


def resnet50(pretrained=False, checkpoints_path="", progress=True, device="cpu", **kwargs):
    """Constructs a ResNet-50 model.
    Args:
        pretrained (bool): If True, returns a model pre-trained on ImageNet
        progress (bool): If True, displays a progress bar of the download to stderr
    """
    return _resnet(
        "resnet50", Bottleneck, [3, 4, 6, 3], pretrained, progress, device, checkpoints_path=checkpoints_path, **kwargs
    )


# class Res50(nn.Module):
#     def __init__(self,  pretrained=True):
#         super(Res50, self).__init__()

#         self.de_pred = nn.Sequential(nn.ReLU(nn.Conv2d(1024, 128, 1)),
#                                      nn.ReLU(nn.Conv2d(128, 1, 1)))

#         self._initialize_weights()
#         res = models.resnet50(pretrained=pretrained)
#         # pre_wts = torch.load(model_path)
#         # res.load_state_dict(pre_wts)
#         self.frontend = nn.Sequential(
#             res.conv1, res.bn1, res.relu, res.maxpool, res.layer1, res.layer2
#         )
#         # self.own_reslayer_3 = self._make_layer(Bottleneck, 256, 64,  6, stride=1)
#         # self.own_reslayer_3.load_state_dict(res.layer3.state_dict())

#     def forward(self,x):
#         print(x.shape)
#         x = self.frontend(x)
#         print(x.shape)
#         # x = self.own_reslayer_3(x)
#         x = self.de_pred(x)
#         x = F.upsample(x,scale_factor=8)
#         return x

#     def _initialize_weights(self):
#         for m in self.modules():
#             if isinstance(m, nn.Conv2d):
#                 m.weight.data.normal_(0.0, std=0.01)
#                 if m.bias is not None:
#                     m.bias.data.fill_(0)
#             elif isinstance(m, nn.BatchNorm2d):
#                 m.weight.fill_(1)
#                 m.bias.data.fill_(0)

    # def _make_layer(self, block, planes, inplanes, blocks, stride=1, dilate=False):
    #     norm_layer = nn.BatchNorm2d
    #     downsample = None
    #     previous_dilation = 1
    #     if dilate:
    #         self.dilation *= stride
    #         stride = 1
    #     if stride != 1 or inplanes != planes * block.expansion:
    #         downsample = nn.Sequential(
    #             conv1x1(inplanes, planes * block.expansion, stride),
    #             norm_layer(planes * block.expansion),
    #         )

    #     layers = []
    #     layers.append(
    #         block(
    #             inplanes,
    #             planes,
    #             stride,
    #             downsample,
    #             self.groups,
    #             self.base_width,
    #             previous_dilation,
    #             norm_layer,
    #         )
    #     )
    #     self.inplanes = planes * block.expansion
    #     for _ in range(1, blocks):
    #         layers.append(
    #             block(
    #                 inplanes,
    #                 planes,
    #                 groups=self.groups,
    #                 base_width=self.base_width,
    #                 dilation=self.dilation,
    #                 norm_layer=norm_layer,
    #             )
    #         )

    #     return nn.Sequential(*layers)

class Conv2d(nn.Module):
    def __init__(self, in_channels, out_channels, kernel_size, stride=1, relu=True, same_padding=False, bn=False):
        super(Conv2d, self).__init__()
        padding = int((kernel_size - 1) / 2) if same_padding else 0
        self.conv = nn.Conv2d(in_channels, out_channels,
                              kernel_size, stride, padding=padding)
        self.bn = nn.BatchNorm2d(
            out_channels, eps=0.001, momentum=0, affine=True) if bn else None
        self.relu = nn.ReLU(inplace=True) if relu else None

    def forward(self, x):
        x = self.conv(x)
        if self.bn is not None:
            x = self.bn(x)
        if self.relu is not None:
            x = self.relu(x)
        return x


class MCNN(nn.Module):
    '''
    Multi-column CNN 
        -Implementation of Single Image Crowd Counting via Multi-column CNN (Zhang et al.)
    '''

    def __init__(self, bn=False):
        super(MCNN, self).__init__()

        self.branch1 = nn.Sequential(Conv2d(3, 16, 9, same_padding=True, bn=bn),
                                     nn.MaxPool2d(2),
                                     Conv2d(
                                         16, 32, 7, same_padding=True, bn=bn),
                                     nn.MaxPool2d(2),
                                     Conv2d(
                                         32, 16, 7, same_padding=True, bn=bn),
                                     Conv2d(16,  8, 7, same_padding=True, bn=bn))

        self.branch2 = nn.Sequential(Conv2d(3, 20, 7, same_padding=True, bn=bn),
                                     nn.MaxPool2d(2),
                                     Conv2d(
                                         20, 40, 5, same_padding=True, bn=bn),
                                     nn.MaxPool2d(2),
                                     Conv2d(
                                         40, 20, 5, same_padding=True, bn=bn),
                                     Conv2d(20, 10, 5, same_padding=True, bn=bn))

        self.branch3 = nn.Sequential(Conv2d(3, 24, 5, same_padding=True, bn=bn),
                                     nn.MaxPool2d(2),
                                     Conv2d(
                                         24, 48, 3, same_padding=True, bn=bn),
                                     nn.MaxPool2d(2),
                                     Conv2d(
                                         48, 24, 3, same_padding=True, bn=bn),
                                     Conv2d(24, 12, 3, same_padding=True, bn=bn))

        self.fuse = nn.Sequential(Conv2d(30, 1, 1, same_padding=True, bn=bn))

    def forward(self, im_data):
        x1 = self.branch1(im_data)
        x2 = self.branch2(im_data)
        x3 = self.branch3(im_data)
        x = torch.cat((x1, x2, x3), 1)
        x = self.fuse(x)

        return x


class DoubleConv(nn.Module):
    def __init__(self, in_channels, out_channels):
        super().__init__()

        # use nn.Sequential(),BatchNorm2d,ReLU. Use padding to keep the size of image.
        self.layers = nn.Sequential(
            nn.Conv2d(in_channels, out_channels,
                      kernel_size=3, padding=1, bias=False),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(inplace=True),
            nn.Conv2d(out_channels, out_channels,
                      kernel_size=3, padding=1, bias=False),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(inplace=True)
        )

    def forward(self, x):
        """
        x: [B, C_in, H, W]
        out: [B, C_out, H, W]
        """
        out = self.layers(x)
        return out


class Down(nn.Module):
    """Downscaling with maxpool then double conv"""

    def __init__(self, in_channels, out_channels):
        super().__init__()
        self.maxpool_conv = nn.Sequential(
            nn.MaxPool2d(2),
            DoubleConv(in_channels, out_channels)
        )

    def forward(self, x):
        x = self.maxpool_conv(x)
        return x


class Up(nn.Module):
    """Upscaling then double conv"""

    def __init__(self, in_channels, out_channels):
        super().__init__()
        self.up = nn.ConvTranspose2d(
            in_channels, in_channels // 2, kernel_size=2, stride=2)
        self.conv = DoubleConv(in_channels, out_channels)

    def forward(self, x1, x2):

        x1 = self.up(x1)
        H1, W1 = x1.shape[2:]
        H2, W2 = x2.shape[2:]
        x1 = F.pad(x1, [
                   (W2-W1) // 2,
                   (W2-W1) // 2,
                   (H2-H1) // 2,
                   (H2-H1) // 2
                   ])
        x = torch.cat([x2, x1], dim=1)
        out = self.conv(x)

        return out


class UNet(nn.Module):
    def __init__(self, n_channels, n_classes, C_base=64):
        super(UNet, self).__init__()
        self.n_channels = n_channels
        self.n_classes = n_classes

        # Use class defined by you, Down() and Up(). Be careful about params here.
        self.in_conv = DoubleConv(n_channels, C_base)

        self.down1 = Down(C_base, C_base*2)
        self.down2 = Down(C_base*2, C_base*4)
        self.down3 = Down(C_base*4, C_base*8)
        self.down4 = Down(C_base*8, C_base*16)
        self.up1 = Up(C_base*16, C_base*8)
        self.up2 = Up(C_base*8, C_base*4)
        self.up3 = Up(C_base*4, C_base*2)
        self.up4 = Up(C_base*2, C_base)

        self.out_projection = nn.Conv2d(C_base, n_classes, kernel_size=1)

    def forward(self, x):
        """
        :param x: [B, n_channels, H, W]
        :return [B, n_classes, H, W]
        """
        x1 = self.in_conv(x)
        x2 = self.down1(x1)
        x3 = self.down2(x2)
        x4 = self.down3(x3)
        x5 = self.down4(x4)
        x = self.up1(x5, x4)
        x = self.up2(x, x3)
        x = self.up3(x, x2)
        x = self.up4(x, x1)
        pred = self.out_projection(x)
        return pred


if __name__ == "__main__":
    # print(resnet32)
    net = UNet(3, 1, 512)
    net.to("cuda")
    # print(net)
    with torch.no_grad():
        outputs = net(torch.randn(4, 3, 512, 512).to("cuda"))
    print(outputs.shape)
