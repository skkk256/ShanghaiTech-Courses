"""
BME1301
DO NOT MODIFY anything in this file.
"""
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

from .utils import imgshow, imsshow, image_mask_overlay, onehot_to_mask
from .evaluation import get_accuracy, get_sensitivity, get_specificity, get_precision, get_F1, get_JS, get_DC, get_DC_square

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

    def _step(self,
              batch: tuple) -> dict:
        raise NotImplementedError()

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
              is_plot=True) -> dict:
        if log:
            torch.cuda.empty_cache()
            timestamp = time.strftime("%m-%d_%H-%M-%S", time.localtime())
            os.mkdir(join('./log', timestamp))
            writer = SummaryWriter(join('./log', timestamp))

        val_loss_epochs = []
        train_loss_epochs = []
        pbar_train = tqdm(total=len(data_loader.sampler), unit='img')
        if val_loader is not None:
            pbar_val = tqdm(total=len(val_loader.sampler),
                            desc=f'[Validation] waiting', unit='img')
        for epoch in range(epochs):
            pbar_train.reset()
            pbar_train.set_description(
                desc=f'[Train] Epoch {epoch + 1}/{epochs}')
            epoch_loss_acc = 0
            epoch_size = 0
            for batch in data_loader:
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
                epoch_loss_acc += loss_value
                epoch_size += batch_size
                pbar_train.update(batch_size)
                pbar_train.set_postfix(loss=loss_value / batch_size)

            epoch_avg_loss = epoch_loss_acc / epoch_size
            # set tensorboard
            if log:
                writer.add_scalar('Train Loss', epoch_avg_loss, epoch)
            pbar_train.set_postfix(epoch_avg_loss=epoch_avg_loss)
            train_loss_epochs.append(epoch_avg_loss)

            if self.lr_scheduler:
                self.lr_scheduler.step()

            # validate if `val_loader` is specified
            if val_loader is not None:
                pbar_val.reset()
                pbar_val.set_description(
                    desc=f'[Validation] Epoch {epoch + 1}/{epochs}')
                val_avg_loss = self.validate(
                    val_loader, pbar=pbar_val, is_compute_metrics=False)
                val_loss_epochs.append(val_avg_loss)
                # output validation image
                if (epoch % 5 == 0):
                    seg_gt_overlay, pred_overlay = self.get_img(0, val_loader)
                    if log:
                        writer.add_image('seg_gt_overlay',
                                        seg_gt_overlay, epoch, dataformats="HWC")
                        writer.add_image('pred_overlay', pred_overlay,
                                        epoch,  dataformats="HWC")
                if log:
                    writer.add_scalar('Validation Loss', val_avg_loss, epoch)

        if log:
            writer.close()
        pbar_train.close()

        if val_loader is not None:
            pbar_val.close()
        train_loss_epochs = torch.tensor(train_loss_epochs).numpy()
        val_loss_epochs = torch.tensor(val_loss_epochs).numpy()
        plt.figure()
        plt.plot(list(range(1, epochs + 1)), train_loss_epochs, label='train')
        if val_loader is not None:
            plt.plot(list(range(1, epochs + 1)),
                     val_loss_epochs, label='validation')
        plt.legend()
        plt.xlabel('Epochs')
        plt.ylabel('Loss')
        plt.show()
        plt.close('all')

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

    def visualize(self, data_loader, idx, net):
        raise NotImplementedError()

    def get_recorder(self) -> dict:
        return self.recorder

    def get_img(self, idx, data_loader):
        with torch.no_grad():
            # fetch data batch
            if idx < 0 or idx > len(data_loader) * data_loader.batch_size:
                raise RuntimeError("idx is out of range.")

            batch_idx = idx // data_loader.batch_size
            batch_offset = idx - batch_idx * data_loader.batch_size

            batch = next(itertools.islice(data_loader, batch_idx, None))

            # inference
            image, seg_gt = batch

            image = self.to_device(image)  # [B, C=1, H, W]
            seg_gt = self.to_device(seg_gt)  # [B, C=1, H, W]
            B, C, H, W = image.shape

            self.model.eval()
            pred_seg = self.model(image)  # [B, C=1, H, W]

            pred_seg_probs = torch.sigmoid(pred_seg)
            pred_seg_mask = pred_seg_probs > 0.5  # default threshoulding: 0.5
            DC = get_DC(pred_seg_probs[batch_offset, ...]
                        [None, ...], seg_gt[batch_offset, ...][None, ...])

            image = self.to_numpy(image[batch_offset, 0, :, :])
            seg_gt_RV = self.to_numpy(seg_gt[batch_offset, 0, :, :])
            seg_gt_MYO = self.to_numpy(seg_gt[batch_offset, 1, :, :])
            seg_gt_LV = self.to_numpy(seg_gt[batch_offset, 2, :, :])
            pred_seg_mask_RV = self.to_numpy(
                pred_seg_mask[batch_offset, 0, :, :])
            pred_seg_mask_MYO = self.to_numpy(
                pred_seg_mask[batch_offset, 1, :, :])
            pred_seg_mask_LV = self.to_numpy(
                pred_seg_mask[batch_offset, 2, :, :])

            # seg_gt_RV = (seg_gt_RV > 0.5) * 1.0
            # seg_gt_MYO = (seg_gt_MYO > 0.5) * 1.0
            # seg_gt_LV = (seg_gt_LV > 0.5) * 1.0

            seg_gt = (seg_gt_RV * 85 + seg_gt_MYO * 170 + seg_gt_LV * 255)/255

            result = np.zeros((256, 256))

            pal = [85, 170, 255]
            for i in range(256):
                for j in range(256):
                    value = np.array(
                        [pred_seg_mask_RV[i, j], pred_seg_mask_MYO[i, j], pred_seg_mask_LV[i, j]])
                    result[i, j] = pal[np.argmax(
                        value)] * value[np.argmax(value)]
            pred_seg_mask = result/255

            # pred_seg_mask = result
            seg_gt_overlay = image_mask_overlay(image, seg_gt)
            pred_overlay = image_mask_overlay(image, pred_seg_mask)

        return seg_gt_overlay, pred_overlay


class Lab2Solver(Solver):
    def _step(self, batch, is_compute_metrics=True) -> dict:
        image, seg_gt = batch

        image = self.to_device(image)  # [B, C=1, H, W]
        seg_gt = self.to_device(seg_gt)  # [B, C=1, H, W]
        B, C, H, W = image.shape

        pred_seg = self.model(image)  # [B, C=1, H, W]
        loss = self.criterion(pred_seg, seg_gt)

        step_dict = {
            'loss': loss,
            'batch_size': B
        }

        # ============ compute metrics TODO
        if not self.model.training and is_compute_metrics:
            pred_seg_probs = torch.sigmoid(pred_seg)
            SE = get_sensitivity(pred_seg_probs, seg_gt)
            SP = get_specificity(pred_seg_probs, seg_gt)
            PC = get_precision(pred_seg_probs, seg_gt)
            F1 = get_F1(pred_seg_probs, seg_gt)
            JS = get_JS(pred_seg_probs, seg_gt)
            DC = get_DC(pred_seg_probs, seg_gt)
            DC_SQUARE = get_DC_square(pred_seg_probs, seg_gt)

            step_dict['metric_avg_Sensitivity'] = SE
            step_dict['metric_avg_Specifity'] = SP
            step_dict['metric_avg_Precision'] = PC
            step_dict['metric_avg_F1Score'] = F1
            step_dict['metric_avg_JaccardSimilarity'] = JS
            step_dict['metric_avg_DiceCoefficient'] = DC
            step_dict['metric_avg_DiceCoefficient_square'] = DC_SQUARE

        return step_dict

    def visualize(self, data_loader, idx, *, dpi=100):
        with torch.no_grad():
            # fetch data batch
            if idx < 0 or idx > len(data_loader) * data_loader.batch_size:
                raise RuntimeError("idx is out of range.")

            batch_idx = idx // data_loader.batch_size
            batch_offset = idx - batch_idx * data_loader.batch_size

            batch = next(itertools.islice(data_loader, batch_idx, None))

            # inference
            image, seg_gt = batch

            image = self.to_device(image)  # [B, C=1, H, W]
            seg_gt = self.to_device(seg_gt)  # [B, C=1, H, W]
            B, C, H, W = image.shape


            self.model.eval()
            pred_seg = self.model(image)  # [B, C=1, H, W]

            pred_seg_probs = torch.sigmoid(pred_seg)
            pred_seg_mask = pred_seg_probs > 0.5  # default threshoulding: 0.5
            DC = get_DC(pred_seg_probs[batch_offset, ...]
                        [None, ...], seg_gt[batch_offset, ...][None, ...])

            image = self.to_numpy(image[batch_offset, 0, :, :])
            seg_gt_RV = self.to_numpy(seg_gt[batch_offset, 0, :, :])
            seg_gt_MYO = self.to_numpy(seg_gt[batch_offset, 1, :, :])
            seg_gt_LV = self.to_numpy(seg_gt[batch_offset, 2, :, :])
            pred_seg_mask_RV = self.to_numpy(
                pred_seg_mask[batch_offset, 0, :, :])
            pred_seg_mask_MYO = self.to_numpy(
                pred_seg_mask[batch_offset, 1, :, :])
            pred_seg_mask_LV = self.to_numpy(
                pred_seg_mask[batch_offset, 2, :, :])

            # seg_gt_RV = (seg_gt_RV > 0.5) * 1.0
            # seg_gt_MYO = (seg_gt_MYO > 0.5) * 1.0
            # seg_gt_LV = (seg_gt_LV > 0.5) * 1.0

            seg_gt = (seg_gt_RV * 85 + seg_gt_MYO * 170 + seg_gt_LV * 255)/255
            # # pred_seg_mask = (pred_seg_mask_RV * 85 + \
            # #     pred_seg_mask_MYO * 170 + pred_seg_mask_LV * 255)/255

            # # max_values = np.maximum.reduce([pred_seg_mask_RV, pred_seg_mask_MYO, pred_seg_mask_LV])
            result = np.zeros((256, 256))

            pal = [85, 170, 255]
            for i in range(256):
                for j in range(256):
                    value = np.array(
                        [pred_seg_mask_RV[i, j], pred_seg_mask_MYO[i, j], pred_seg_mask_LV[i, j]])
                    result[i, j] = pal[np.argmax(
                        value)] * value[np.argmax(value)]
            pred_seg_mask = result/255

            seg_gt_overlay = image_mask_overlay(image, seg_gt)
            pred_overlay = image_mask_overlay(image, pred_seg_mask)

        imsshow([image, seg_gt, pred_seg_mask],
                titles=['Image',
                        f"Segmentation GT",
                        f"Prediction (DICE {DC:.2f})"],
                num_col=3,
                dpi=dpi,
                is_colorbar=True)
        imsshow([seg_gt_overlay, pred_overlay],
                titles=[f"Segmentation GT",
                        f"Prediction (DICE {DC:.2f})"],
                num_col=2,
                dpi=dpi,
                is_colorbar=False)

    def inference_all(self, data_loader, output_path) -> None:
        torch.cuda.empty_cache()

        with torch.no_grad():
            self.model.eval()
            for batch in tqdm(data_loader):
                image, filename = batch
                B, C, H, W = image.shape

                image = self.to_device(image)  # [B, C=1, H, W]

                pred_seg = self.model(image)  # [B, C=1, H, W]

                pred_seg_probs = torch.sigmoid(pred_seg)
                pred_seg_mask = pred_seg_probs > 0.5  # default threshoulding: 0.5
                pred_seg_mask = pred_seg_mask * 1.0
                pred_seg_mask = pred_seg_mask.cpu()  # [B, C=1, H, W]

                for i in range(B):
                    torchvision.utils.save_image(
                        pred_seg_mask[i, 0, :, :],
                        os.path.join(
                            output_path, f'case_{filename[i]}_segmentation.jpg')
                    )
