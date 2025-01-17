from torch import nn
from torch.nn import functional as F
import torch


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

        # use nn.Sequential,nn.MaxPool2d and DoubleConv defined by you.
        ######################## WRITE YOUR ANSWER BELOW ########################
        self.maxpool_conv = nn.Sequential(
            nn.MaxPool2d(2),
            DoubleConv(in_channels, out_channels)
        )
        #########################################################################

    def forward(self, x):
        ######################## WRITE YOUR ANSWER BELOW ########################
        x = self.maxpool_conv(x)
        return x
        #########################################################################


class Up(nn.Module):
    """Upscaling then double conv"""

    def __init__(self, in_channels, out_channels):
        super().__init__()

        # use nn.ConvTranspose2d for upsampling.
        ######################## WRITE YOUR ANSWER BELOW ########################
        self.up = nn.ConvTranspose2d(
            in_channels, in_channels // 2, kernel_size=2, stride=2)
        #########################################################################
        self.conv = DoubleConv(in_channels, out_channels)

    def forward(self, x1, x2):

        x1 = self.up(x1)
        H1, W1 = x1.shape[2:]
        H2, W2 = x2.shape[2:]

        # print('before padding:', x1.shape)
        # temp = torch.clone(x1)
        # use F.pad to change the shape of x1.
        ######################## WRITE YOUR ANSWER BELOW ########################
        x1 = F.pad(x1, [
                   (W2-W1) // 2,
                   (W2-W1) // 2,
                   (H2-H1) // 2,
                   (H2-H1) // 2
                   ])
        #########################################################################
        # print('after padding: ',x1.shape)
        # print(torch.equal(x1, temp))  # True

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
