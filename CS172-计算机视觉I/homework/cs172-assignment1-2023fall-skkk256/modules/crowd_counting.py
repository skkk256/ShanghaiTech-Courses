import torch
import cv2
import numpy as np
import time
import os
from os.path import join

import torch.optim as optim
from torch import nn
from torch.utils.data import DataLoader
from typing import Any, Callable
from argparse import ArgumentParser
from tqdm.autonotebook import tqdm  # may raise warning about Jupyter
from tqdm.auto import tqdm  # who needs warnings
import matplotlib.pyplot as plt
from torch.utils.tensorboard import SummaryWriter

from modules.utils import get_density_map_gaussian


def data_collate(batch):
    data = [item[0] for item in batch]
    target = [item[1] for item in batch]
    data = torch.stack(data, 0)
    return [data, target]


def draw_and_save(images, coords, save_path, batch_idx, model_name, timestamp):
    std = torch.tensor([0.229, 0.224, 0.225])
    mean = torch.tensor([0.485, 0.456, 0.406])
    for i in range(images.shape[0]):
        image = images[i].permute((1, 2, 0))
        image = image * std + mean
        image = image.numpy()
        coord = coords[i]
        fig, ax = plt.subplots(1)
        ax.imshow(image)
        ax.plot(coord[:, 0], coord[:, 1], 'ro')
        plt.savefig(
            f"{save_path}/{model_name}_{timestamp}_image_{batch_idx+i}.png")
        plt.close()


class Task2_Solver(object):
    def __init__(self,
                 model: nn.Module,
                 optimizer: torch.optim.Optimizer,
                 criterion: Callable,
                 lr_scheduler=None,
                 model_name: str = "",
                 downsample: bool = True,
                 recorder: dict = None,
                 device=None):
        device = device if device is not None else \
            ('cuda' if torch.cuda.is_available() else 'cpu')
        self.device = device
        self.recorder = recorder

        self.model_name = model_name
        self.downsample = downsample
        self.model = self.to_device(model)
        self.optimizer = optimizer
        self.criterion = criterion
        self.lr_scheduler = lr_scheduler

    def save_checkpoints(self, path):
        torch.save(self.model.state_dict(), path)

    def load_checkpoints(self, path):
        self.model.load_state_dict(torch.load(path))

    def to_device(self, x):
        if isinstance(x, torch.Tensor):
            return x.to(self.device)
        elif isinstance(x, np.ndarray):
            return torch.tensor(x, device=self.device)
        elif isinstance(x, nn.Module):
            return x.to(self.device)
        else:
            raise RuntimeError("Data cannot transfer to correct device.")

    def to_numpy(self, x):
        if isinstance(x, np.ndarray):
            return x
        elif isinstance(x, torch.Tensor):
            return x.detach().cpu().numpy()
        else:
            raise RuntimeError(
                f"Cannot convert type {type(x)} into numpy array.")

    def train(self,
              epochs: int,
              data_loader,
              log: bool,
              *,
              val_loader=None,
              save_per_epoch=None,
              is_plot=True):
        timestamp = time.strftime("%m-%d_%H-%M-%S", time.localtime())
        if log:
            torch.cuda.empty_cache()
            os.mkdir(join('./log', f"{self.model_name}_{timestamp}"))
            writer = SummaryWriter(
                join('./log', f"{self.model_name}_{timestamp}"))
            # Create Visualization folder
            if not os.path.exists("./train_images"):
                os.makedirs("./train_images")
            if not os.path.exists("./test_images"):
                os.makedirs("./test_images")

        running_loss = 0
        self.model.train()
        pbar_train = tqdm(total=len(data_loader.sampler), unit='img')
        for epoch in range(epochs):
            pbar_train.reset()
            pbar_train.set_description(
                desc=f'[Train] Epoch {epoch + 1}/{epochs}')
            for batch_idx, inputs in enumerate(data_loader):
                images, gt = inputs

                # Visualize data, you can delete this part if you don't want to visualize the data
                # draw_and_save(images, gt, "./train_images",
                #             batch_idx*args.batch_size)
                images, gt = inputs
                B, C, H, W = images.shape

                density_maps = []
                for i in range(len(images)):
                    density_map = get_density_map_gaussian(
                        images[i][0], gt[i].numpy())
                    if self.downsample:
                        density_map = cv2.resize(density_map, (128, 128))
                    density_maps.append(torch.from_numpy(density_map * 2000))

                images = images.to(self.device)
                density_maps = torch.stack(
                    density_maps).unsqueeze(1).to(self.device)

                #  Forward
                self.optimizer.zero_grad()
                outputs = self.model(images)
                loss = self.criterion(outputs, density_maps)
                #  Backward
                loss.backward()
                #  Update parameters
                self.optimizer.step()
                #  Print log info
                print(
                    f"actual number is {float(gt[0].shape[0])}, predict is {float(torch.sum(outputs.cpu()[0][0]) / 2000 * 16)}")

                loss_value = loss.item()
                running_loss += loss_value
                pbar_train.update(B)
                pbar_train.set_postfix(loss=loss_value)

            pbar_train.set_postfix(
                epoch_avg_loss=running_loss/len(data_loader.sampler))
            if log:
                writer.add_scalar('train loss per epoch',
                                  running_loss/len(data_loader.sampler), epoch+1)
            running_loss = 0.0
            if epoch % save_per_epoch == save_per_epoch - 1:
                self.save_checkpoints(
                    f"checkpoints/{self.model_name}_{timestamp}_{epoch+1}.pth")
                if val_loader:
                    self.test(val_loader, epoch, timestamp)
        # Save model checkpoints

        # for batch_idx, inputs in enumerate(val_loader):
        #     images, gt = inputs
        #     # Visualize data, you can delete this part if you don't want to visualize the data
        #     draw_and_save(images, gt, "./test_images", batch_idx)

        #     # TODO Test model performance
    def evaluate_model(self, data_loader):
        # self.load_checkpoints(trained_model)
        mae = 0.0
        mse = 0.0
        with torch.no_grad():
            for blob in data_loader:
                images, gt = blob
                
                density_maps = []
                for i in range(len(images)):
                    density_map = get_density_map_gaussian(
                        np.transpose(images[i], (1, 2, 0)), gt[i].numpy())
                    if self.downsample:
                        density_map = cv2.resize(density_map, (128, 128))
                    density_maps.append(torch.from_numpy(density_map * 2000))

                images = images.to(self.device)
                density_maps = torch.stack(
                    density_maps).unsqueeze(1).to(self.device)
                outputs = self.model(images)
                gt_count = torch.sum(density_maps) / 2000 * 16 #归一化
                et_count = torch.sum(outputs) / 2000 * 16
                
                # density_map = self.model(im_data, gt_data)
                # density_map = density_map.data.cpu().numpy()
                # gt_count = np.sum(gt_data)
                # et_count = np.sum(density_map)
                mae += abs(gt_count-et_count)
                mse += ((gt_count-et_count)*(gt_count-et_count))
                
            mae = mae/len(data_loader.sampler)
            mse = torch.sqrt(mse/len(data_loader.sampler))
            return mae, mse

    def test(self, data_loader, epoch, timestamp):
        torch.cuda.empty_cache()
        loss_acc = 0
        size_acc = 0
        self.model.eval()
        # since we're not training, we don't need to calculate the gradients for our outputs
        with torch.no_grad():
            pbar = tqdm(total=len(data_loader.sampler),
                        desc=f'[Validation]', unit='img')
            for batch_idx, inputs in enumerate(data_loader):
                images, gt = inputs
                B, C, H, W = images.shape

                density_maps = []
                for i in range(len(images)):
                    density_map = get_density_map_gaussian(
                        np.transpose(images[i], (1, 2, 0)), gt[i].numpy())
                    if self.downsample:
                        density_map = cv2.resize(density_map, (128, 128))
                    density_maps.append(torch.from_numpy(density_map * 2000))

                images = images.to(self.device)
                density_maps = torch.stack(
                    density_maps).unsqueeze(1).to(self.device)
                outputs = self.model(images)
                loss = self.criterion(outputs, density_maps)
                loss_acc += loss.item()
                size_acc += B
                pbar.update(B)
                pbar.set_postfix(loss=loss.item())

                if batch_idx % 5 == 4:
                    fg, (ax0, ax1, ax2) = plt.subplots(1, 3, figsize=(16, 4))
                    ax0.imshow(np.transpose(images.cpu()[0], (1, 2, 0)))
                    ax0.set_title(str(gt[0].shape[0]))

                    ax1.imshow(outputs.cpu()[0][0], cmap=plt.cm.jet)
                    ax1.set_title(
                        str(float(torch.sum(outputs.cpu()[0][0]) / 2000 * 16)))

                    ax2.imshow(density_maps.cpu()[0][0], cmap=plt.cm.jet)
                    ax2.set_title(
                        str(float(torch.sum(density_maps.cpu()[0][0]) * 16 / 2000)))
                    print(
                        f"actual number of person is {float(gt[0].shape[0])}, predict is {float(torch.sum(outputs.cpu()[0][0]) * 16 / 2000)}")

                    plt.savefig(
                        f"test_images/{self.model_name}_{timestamp}_image_{epoch}_{batch_idx}.jpg", format='jpeg', dpi=300)

            val_avg_loss = loss_acc / size_acc
            pbar.set_postfix(val_avg_loss=val_avg_loss)

    def __call__(self, input, *args: Any, **kwds: Any) -> Any:
        self.model.eval()
        with torch.no_grad():
            return self.model(input)
