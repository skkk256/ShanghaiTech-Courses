import os
from os.path import join
import time
import itertools
import statistics
from typing import Callable

import numpy as np

# from tqdm import tqdm
from matplotlib import pyplot as plt
from tqdm.autonotebook import tqdm  # may raise warning about Jupyter
from tqdm.auto import tqdm  # who needs warnings

import torch
import torchvision
from torch import nn
from torch.utils import data as Data
from torch.utils.tensorboard import SummaryWriter


class Solver(object):
    def __init__(self,
                 model: nn.Module,
                 optimizer: torch.optim.Optimizer,
                 criterion: Callable,
                 lr_scheduler=None,
                 recorder: dict = None,
                 device=None):
        device = device if device is not None else \
            ('cuda:0' if torch.cuda.is_available() else 'cpu')
        self.device = device
        self.recorder = recorder

        self.model = self.to_device(model)
        self.optimizer = optimizer
        self.criterion = criterion
        self.lr_scheduler = lr_scheduler

    def save_checkpoints(self, path):
        torch.save(self.model.state_dict(), path)

    def load_checkpoints(self, path):
        self.model.load_state_dict(torch.load(path))

    def _step(self, batch, is_compute_metrics=True) -> dict:
        image, gt = batch

        image = self.to_device(image)  # [B, C=3, H, W]
        gt = self.to_device(gt)  # [B, C=3, H, W]
        B, C, H, W = image.shape

        outputs = self.model(image)  # [B, C=1, H, W]
        loss = self.criterion(outputs, gt)

        step_dict = {
            'loss': loss,
            'batch_size': B
        }

        return step_dict

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
              timestamp=None,
              is_plot=True) -> dict:
        if log:
            torch.cuda.empty_cache()
            timestamp = time.strftime("%m-%d_%H-%M-%S", time.localtime())
            os.mkdir(join('./log', timestamp))
            writer = SummaryWriter(join('./log', timestamp))

        val_loss_epochs = []
        train_loss_epochs = []
        running_loss = 0
        step = 0
        pbar_train = tqdm(total=len(data_loader.sampler), unit='img')
        # if val_loader is not None:
        #     pbar_val = tqdm(total=len(val_loader.sampler),
        #                     desc=f'[Validation] waiting', unit='img')
        for epoch in range(epochs):
            pbar_train.reset()
            pbar_train.set_description(
                desc=f'[Train] Epoch {epoch + 1}/{epochs}')
            for idx, batch in enumerate(data_loader, 0):
                self.model.train()
                # forward
                step_dict = self._step(batch)
                batch_size = step_dict['batch_size']
                loss = step_dict['loss']

                # backward
                self.optimizer.zero_grad()
                loss.backward()

                # optimize
                self.optimizer.step()

                # update information
                loss_value = loss.item()
                
                # epoch_loss_acc += loss_value
                pbar_train.update(batch_size)
                pbar_train.set_postfix(loss=loss_value / batch_size)
                running_loss += loss.item()
                step += 1
                
                if idx % 1000 == 999:    # print every 2000 mini-batches
                    print(f'[{epoch + 1}, {idx + 1:5d}] loss: {running_loss / 1000:.3f}')
                    # pbar_train.set_postfix(loss=running_loss / 1000)
                    pbar_train.set_postfix(epoch_avg_loss=running_loss)
                    train_loss_epochs.append(running_loss)
                    if log:
                        writer.add_scalar('train loss per 1000 batches', running_loss/1000, step+1)
                    running_loss = 0.0
                    
            if save_per_epoch and epoch % save_per_epoch == save_per_epoch - 1:
                self.test(val_loader)
                print(f"{epoch+1} checkpoint saved")
                self.save_checkpoints(f"checkpoints/{timestamp}_{epoch+1}.pth")

            # epoch_avg_loss = epoch_loss_acc / epoch_size
            # set tensorboard

            if self.lr_scheduler:
                self.lr_scheduler.step()

        if log:
            writer.close()
        pbar_train.close()

        # if val_loader is not None:
        #     pbar_val.close()
        train_loss_epochs = torch.tensor(train_loss_epochs).numpy()
        val_loss_epochs = torch.tensor(val_loss_epochs).numpy()
        # plt.figure()
        # plt.plot(list(range(1, len(train_loss_epochs) + 1)), train_loss_epochs, label='train')
        # # if val_loader is not None:
        # #     plt.plot(list(range(1, epochs + 1)),
        # #              val_loss_epochs, label='validation')
        # plt.legend()
        # plt.xlabel('Epochs')
        # plt.ylabel('Loss')
        # plt.show()
        # plt.close('all')

    def validate(self, data_loader, *, pbar=None, is_compute_metrics=True) -> float:
        """
        :param pbar: when pbar is specified, do not print average loss
        :return:
        """
        torch.cuda.empty_cache()

        metrics_acc = {}
        loss_acc = 0
        size_acc = 0
        is_need_log = (pbar is None)
        with torch.no_grad():
            if pbar is None:
                pbar = tqdm(total=len(data_loader.sampler),
                            desc=f'[Validation]', unit='img')
            for batch in data_loader:
                self.model.eval()

                # forward
                step_dict = self._step(
                    batch, is_compute_metrics=is_compute_metrics)
                batch_size = step_dict['batch_size']
                loss = step_dict['loss']
                loss_value = loss.item()

                # aggregate metrics
                metrics_acc = self._aggregate_metrics(metrics_acc, step_dict)

                # update information
                loss_acc += loss_value
                size_acc += batch_size
                pbar.update(batch_size)
                pbar.set_postfix(loss=loss_value)

        val_avg_loss = loss_acc / size_acc
        pbar.set_postfix(val_avg_loss=val_avg_loss)
        if is_need_log:
            pbar.close()  # destroy newly created pbar
            print('=' * 30 + ' Measurements ' + '=' * 30)
            for k, v in metrics_acc.items():
                if k == "metric_avg_DiceCoefficient_square":
                    print(
                        f"[metric_avg_DiceCoefficient_sd] {v - step_dict['metric_avg_DiceCoefficient']}")
                else:
                    print(f"[{k}] {v / size_acc}")
        else:
            return val_avg_loss

    def _aggregate_metrics(self, metrics_acc: dict, step_dict: dict):
        batch_size = step_dict['batch_size']
        for k, v in step_dict.items():
            if k[:7] == 'metric_':
                value = v * batch_size
                metric_name = k[7:]
                if metric_name not in metrics_acc:
                    metrics_acc[metric_name] = value
                else:
                    metrics_acc[metric_name] += value
        return metrics_acc

    def test(self, data_loader):
        correct = 0
        total = 0
        self.model.eval()
        # since we're not training, we don't need to calculate the gradients for our outputs
        with torch.no_grad():
            for data in tqdm(data_loader):
                images, labels = data
                images = images.to(self.device)
                labels = labels.to(self.device)
                # calculate outputs by running images through the network
                outputs = self.model(images)
                # the class with the highest energy is what we choose as prediction
                _, predicted = torch.max(outputs.data, 1)
                total += labels.size(0)
                correct += (predicted == labels).sum().item()

        print(
            f'Accuracy of the network on the 10000 test images: {100 * correct // total} %')
