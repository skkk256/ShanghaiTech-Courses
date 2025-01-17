import os
from scipy import io as sio
import torch
from torch.utils import data
import torchvision.transforms.functional as F
from torchvision import transforms
from PIL import Image
import numpy as np
import random


def random_crop(im_h, im_w, crop_h, crop_w):
    res_h = im_h - crop_h
    res_w = im_w - crop_w
    i = random.randint(0, res_h)
    j = random.randint(0, res_w)
    return i, j, crop_h, crop_w


class SHHA_loader(data.Dataset):
    def __init__(self, data_path, split, output_size=256):
        self.data_path = os.path.join(data_path, split + "_data")
        self.img_path = os.path.join(self.data_path, "images")
        self.gt_path = os.path.join(self.data_path, "ground_truth")

        self.file_names = [filename for filename in os.listdir(self.img_path)
                           if os.path.isfile(os.path.join(self.img_path, filename))]

        self.output_size = output_size
        self.split = split
        assert self.split in ["train", "test"]

        self.test = transforms.Compose([
            transforms.ToTensor()
        ])

        if self.split == "train":
            self.trans = transforms.Compose([
                transforms.ToTensor(),
                transforms.Normalize([0.485, 0.456, 0.406], [
                                     0.229, 0.224, 0.225])
            ])
        else:
            self.trans = transforms.Compose([
                transforms.Resize((self.output_size, self.output_size)),
                transforms.ToTensor(),
                transforms.Normalize([0.485, 0.456, 0.406], [
                                     0.229, 0.224, 0.225])
            ])

    def __getitem__(self, index):
        image_name = self.file_names[index]
        gt_name = "GT_" + image_name.replace('.jpg', '.mat')
        image = Image.open(os.path.join(
            self.img_path, image_name)).convert('RGB')
        gt = sio.loadmat(os.path.join(self.gt_path, gt_name))
        gt = gt["image_info"][0, 0][0, 0][0] - 1
        if self.split == "train":
            return self.transform_RC(image, gt)
        else:
            return self.transform_resize(image, gt)

    def __len__(self):
        return len(self.file_names)

    def transform_resize(self, img, keypoints):
        wd, ht = img.size
        # print(keypoints.shape)
        keypoints[:, 0] *= self.output_size / wd
        keypoints[:, 1] *= self.output_size / ht
        img_trans = self.trans(img),
        i, j, w, h = 0, 0, 512, 512
        mask = (keypoints[:, 0] >= j) & (keypoints[:, 0] < j + w) & \
               (keypoints[:, 1] >= i) & (keypoints[:, 1] < i + h)
        keypoints = keypoints[mask]
        keypoints[:, 0] = keypoints[:, 0] - j
        keypoints[:, 1] = keypoints[:, 1] - i
        gt = torch.from_numpy(keypoints.copy()).float()
        return img_trans[0], gt

    def transform_RC(self, img, keypoints):
        """random crop image patch and find people in it"""
        wd, ht = img.size
        st_size = min(wd, ht)

        min_factor = max(self.output_size / st_size, 0.5)
        if min_factor > 1.5:
            factor = min_factor
        else:
            factor = np.random.uniform(low=min_factor, high=1.5)

        img = F.resize(img, (int(ht * factor), int(wd * factor)))
        keypoints = keypoints * factor
        wd, ht = img.size

        i, j, h, w = random_crop(ht, wd, self.output_size, self.output_size)
        img = F.crop(img, i, j, h, w)

        mask = (keypoints[:, 0] >= j) & (keypoints[:, 0] < j + w) & \
               (keypoints[:, 1] >= i) & (keypoints[:, 1] < i + h)
        keypoints = keypoints[mask]
        keypoints[:, 0] = keypoints[:, 0] - j
        keypoints[:, 1] = keypoints[:, 1] - i

        if len(keypoints) > 0:
            if random.random() > 0.5:
                img = F.hflip(img)
                keypoints[:, 0] = w - keypoints[:, 0]
        else:
            if random.random() > 0.5:
                img = F.hflip(img)

        img_trans = self.trans(img)
        gt = torch.from_numpy(keypoints.copy()).float()
        # print(img_trans.shape)
        return img_trans, gt
