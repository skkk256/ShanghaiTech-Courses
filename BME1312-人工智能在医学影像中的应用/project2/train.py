import numpy as np
from matplotlib import pyplot as plt
import torch
from torch import nn
from torch.nn import functional as F
from torch.utils import data
from torchvision import transforms
import os
import random
from PIL import Image

from modules.dataset import get_loader
from modules.utils import imsshow, imgshow
from modules.network import UNet
from modules.loss import MyBinaryCrossEntropy
from modules.solver import Lab2Solver


RV = [85]
MYO = [170]
LV = [255]


if __name__ == "__main__":
    train_loader = get_loader(image_root_path='./ACDC-2D-All/train/',
                        palette=[RV, MYO, LV], batch_size=32, mode='train')
    val_loader = get_loader(image_root_path='./ACDC-2D-All/val/',
                            palette=[RV, MYO, LV], batch_size=32, mode='val')
    test_loader = get_loader(image_root_path='./ACDC-2D-All/test/',
                            palette=[RV, MYO, LV], batch_size=32, mode='test')

    net = UNet(n_channels=1, n_classes=3, C_base=32)
    optimizer = torch.optim.Adam(net.parameters(), lr=0.01)
    lr_scheduler = torch.optim.lr_scheduler.ExponentialLR(
        optimizer=optimizer, gamma=0.95)

    solver = Lab2Solver(
        model=net,
        optimizer=optimizer,
        criterion=MyBinaryCrossEntropy(),
        lr_scheduler=lr_scheduler,
        device="cuda:2"
    )

    solver.train(
    epochs=50, 
    data_loader=train_loader,
    val_loader=val_loader
)
