from __future__ import absolute_import, division, print_function
import numpy as np
import torch
import torch.nn as nn
import torchvision.models as models
import torch.utils.model_zoo as model_zoo

from .resnet import resnet18, resnet34, resnet50, resnet101

# model_urls = {
#     'resnet18': 'https://download.pytorch.org/models/resnet18-5c106cde.pth',
#     'resnet34': 'https://download.pytorch.org/models/resnet34-333f7ec4.pth',
#     'resnet50': 'https://download.pytorch.org/models/resnet50-19c8e357.pth',
#     'resnet101': 'https://download.pytorch.org/models/resnet101-5d3b4d8f.pth',
#     'resnet152': 'https://download.pytorch.org/models/resnet152-b121ed2d.pth',
#     'resnext50_32x4d': 'https://download.pytorch.org/models/resnext50_32x4d-7cdf4587.pth',
#     'resnext101_32x8d': 'https://download.pytorch.org/models/resnext101_32x8d-8ba56ff5.pth',
#     'wide_resnet50_2': 'https://download.pytorch.org/models/wide_resnet50_2-95faca4d.pth',
#     'wide_resnet101_2': 'https://download.pytorch.org/models/wide_resnet101_2-32ee1156.pth',
# }

class FeatureEncoder(nn.Module):
    def __init__(self, num_layers, pretrained_path=None):
        super(FeatureEncoder, self).__init__()

        self.num_ch_enc = np.array([64, 64, 128, 256, 512])

        resnets = {18: resnet18,
                   34: resnet34,
                   50: resnet50,
                   101: resnet101,}
        # resnets = model_zoo.load_url(model_urls['resnet{}'.format(num_layers)])

        if num_layers not in resnets:
            raise ValueError("{} is not a valid number of resnet layers".format(num_layers))


        self.encoder = resnets[num_layers]()
        if pretrained_path is not None:
            checkpoint = torch.load(pretrained_path)
            self.encoder.load_state_dict(checkpoint)

        if num_layers > 34:
            self.num_ch_enc[1:] *= 4

        # for name, param in self.encoder.named_parameters():
        #     if 'bn' in name:
        #         param.requires_grad = False

    def forward(self, input_image):
        self.features = []
        self.features.append(self.encoder.relu(self.encoder.bn1(self.encoder.conv1(input_image))))
        self.features.append(self.encoder.layer1(self.encoder.maxpool(self.features[-1])))
        self.features.append(self.encoder.layer2(self.features[-1]))
        self.features.append(self.encoder.layer3(self.features[-1]))
        self.features.append(self.encoder.layer4(self.features[-1]))

        return self.features
