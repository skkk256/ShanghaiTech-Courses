from torchvision import transforms as T
from torchvision.transforms import functional as F
from torch.utils import data
import torch
import os
import random
import numpy as np
from PIL import Image

from .utils import mask_to_onehot


def get_loader(
    image_root_path, batch_size, palette=[[85], [170], [255]], num_workers=0, mode="train", augmentation_prob=0.4
):
    """Builds and returns Dataloader."""
    if mode == "test" or mode == "val":
        dataset = Test_ImageFolder(
            root=image_root_path, palette=palette, mode=mode, augmentation_prob=augmentation_prob
        )
    else:
        dataset = ImageFolder(
            root=image_root_path, palette=palette, mode=mode, augmentation_prob=augmentation_prob
        )

    data_loader = data.DataLoader(
        dataset=dataset,
        batch_size=batch_size,
        shuffle=True if mode == "train" else False,
        num_workers=num_workers,
    )

    return data_loader


class ImageFolder(data.Dataset):
    def __init__(self, root, palette=[[85], [170], [255]], mode="train", augmentation_prob=0.4):
        assert mode in {"train"}
        """Initializes image paths and preprocessing module."""
        self.root = root
        self.palette = palette

        # GT : Ground Truth
        self.GT_dir = os.path.join(root, "GT/")
        self.image_names = os.listdir(self.GT_dir)
        self.len = len(self.image_names)
        self.GT_paths = [
            os.path.join(self.GT_dir, self.image_names[i]) for i in range(self.len)
        ]

        # Generate the paths of input images.

        self.image_dir = os.path.join(root, "Img/")
        self.image_paths = [
            os.path.join(self.image_dir, self.image_names[i]) for i in range(self.len)
        ]

        self.mode = mode
        self.RotationDegree = [0, 90, 180, 270]
        self.augmentation_prob = augmentation_prob
        print("image count in {} path :{}".format(self.mode, self.len))

    def __getitem__(self, index):
        """Reads an image from a file and preprocesses it and returns."""
        image_path = self.image_paths[index]
        filename = self.image_names[index]
        GT_path = self.GT_paths[index]

        image = Image.open(image_path)
        seg_gt = Image.open(GT_path)
        # # print(f"the size of img and gt init is {image.size} and {seg_gt.size}")
        # seg_gt = np.expand_dims(seg_gt, axis=0)
        # # print(f"the size of img and gt after expand is {image.size} and {seg_gt.size}")
        # seg_gt = mask_to_onehot(seg_gt, self.palette)
        # # print(f"the size of img and gt is {image.shape} and {seg_gt.shape}")
        # seg_gt = [Image.fromarray(np.uint8(seg_gt[i] * 255)) for i in range(3)]

        aspect_ratio = image.size[1] / image.size[0]

        Transform = []

        ResizeRange = random.randint(300, 320)
        Transform.append(
            T.Resize((int(ResizeRange * aspect_ratio), ResizeRange), interpolation=F.InterpolationMode.NEAREST))
        p_transform = random.random()

        if (self.mode == "train") and p_transform <= self.augmentation_prob:

            RotationDegree = random.randint(0, 3)
            RotationDegree = self.RotationDegree[RotationDegree]
            if (RotationDegree == 90) or (RotationDegree == 270):
                aspect_ratio = 1 / aspect_ratio

            Transform.append(T.RandomRotation(
                (RotationDegree, RotationDegree)))

            # use randint and T.RandomRotation to rotate the image by (-10,10) degrees.

            RotationRange = random.randint(-10, 10)
            Transform.append(T.RandomRotation((RotationRange, RotationRange)))

            CropRange = random.randint(250, 270)
            Transform.append(T.CenterCrop(
                (int(CropRange * aspect_ratio), CropRange)))
            Transform = T.Compose(Transform)

            image = Transform(image)

            # Be careful: when you do geometric transformation on the original image,you need to do
            # the same transform on the gt, to keep the consistency.
            seg_gt = Transform(seg_gt)

            ShiftRange_left = random.randint(0, 20)
            ShiftRange_upper = random.randint(0, 20)
            ShiftRange_right = image.size[0] - random.randint(0, 20)
            ShiftRange_lower = image.size[1] - random.randint(0, 20)
            image = image.crop(
                box=(
                    ShiftRange_left,
                    ShiftRange_upper,
                    ShiftRange_right,
                    ShiftRange_lower,
                )
            )
            seg_gt = seg_gt.crop(
                box=(
                    ShiftRange_left,
                    ShiftRange_upper,
                    ShiftRange_right,
                    ShiftRange_lower,
                )
            )

            if random.random() < 0.5:
                image = F.hflip(image)
                seg_gt = F.hflip(seg_gt)

            if random.random() < 0.5:
                image = F.vflip(image)
                seg_gt = F.vflip(seg_gt)

            # use T.ColorJitter to do color transform here. You can't change the color
            # of gt! Set brightness=0.2, contrast=0.2, hue=0.02.

            Transform = T.ColorJitter(brightness=0.2, contrast=0.2, hue=0.02)

            image = Transform(image)

            Transform = []

        Transform.append(
            T.Resize((int(256 * aspect_ratio) -
                     int(256 * aspect_ratio) % 16, 256), interpolation=F.InterpolationMode.NEAREST)
        )
        # Transform.append(T.ToTensor())
        Transform = T.Compose(Transform)

        image = Transform(image)
        seg_gt = Transform(seg_gt)
        # seg_gt = torch.cat([seg_gt[0], seg_gt[1], seg_gt[2]], dim=0)
        # print(f"the size of img and gt is {image.size} and {seg_gt.size}")

        seg_gt = np.expand_dims(seg_gt, axis=-1)
        # print(f"the size of img and gt after expand is {image.size} and {seg_gt.shape}")
        seg_gt = mask_to_onehot(seg_gt, self.palette)
        # print(f"the size of img and gt after mask is {image.size} and {seg_gt.shape}")
        # # print(f"the size of img and gt is {image.shape} and {seg_gt.shape}")

        tensor_trans = T.ToTensor()
        seg_gt = tensor_trans(seg_gt)
        image = tensor_trans(image)

        Norm_ = T.Normalize((0.5), (0.5))
        image = Norm_(image)

        return image, seg_gt

    def __len__(self):
        """Returns the total number of font files."""
        return self.len


class Test_ImageFolder(data.Dataset):
    def __init__(self, root, mode="train", palette=[[85], [170], [255]], augmentation_prob=0.4):
        """Initializes image paths and preprocessing module."""
        assert mode in {"val", "test"}
        self.root = root
        self.palette = palette

        self.GT_dir = os.path.join(root, "GT/")
        self.image_names = os.listdir(self.GT_dir)
        self.len = len(self.image_names)
        self.GT_paths = [
            os.path.join(self.GT_dir, self.image_names[i]) for i in range(self.len)
        ]

        self.image_dir = os.path.join(root, "Img/")
        self.image_paths = [
            os.path.join(self.image_dir, self.image_names[i]) for i in range(self.len)
        ]

        self.mode = mode
        self.RotationDegree = [0, 90, 180, 270]
        self.augmentation_prob = augmentation_prob
        print("image count in {} path :{}".format(
            self.mode, len(self.image_paths)))

    def __getitem__(self, index):
        """Reads an image from a file and preprocesses it and returns."""
        image_path = self.image_paths[index]
        filename = self.image_names[index]
        GT_path = self.GT_paths[index]

        image = Image.open(image_path)
        seg_gt = Image.open(GT_path)
        # # print(f"the size of img and gt init is {image.size} and {seg_gt.size}")
        # seg_gt = np.expand_dims(seg_gt, axis=0)
        # # print(f"the size of img and gt after expand is {image.size} and {seg_gt.size}")
        # seg_gt = mask_to_onehot(seg_gt, self.palette)
        # # print(f"the size of img and gt is {image.shape} and {seg_gt.shape}")
        # seg_gt = [Image.fromarray(np.uint8(seg_gt[i] * 255)) for i in range(3)]

        aspect_ratio = image.size[1] / image.size[0]

        Transform = []
        ResizeRange = random.randint(300, 320)
        Transform.append(
            T.Resize((int(ResizeRange * aspect_ratio), ResizeRange), interpolation=F.InterpolationMode.NEAREST))

        Transform.append(
            T.Resize((int(256 * aspect_ratio) -
                     int(256 * aspect_ratio) % 16, 256), interpolation=F.InterpolationMode.NEAREST)
        )
        # Transform.append(T.ToTensor())
        Transform = T.Compose(Transform)

        image = Transform(image)
        seg_gt = Transform(seg_gt)
        # seg_gt = torch.cat([seg_gt[0], seg_gt[1], seg_gt[2]], dim=0)

        # split gt into 3 channel
        seg_gt = np.expand_dims(seg_gt, axis=-1)
        # print(f"the size of img and gt after expand is {image.size} and {seg_gt.shape}")
        seg_gt = mask_to_onehot(seg_gt, self.palette)

        # transform image and gt to tensor
        tensor_trans = T.ToTensor()
        seg_gt = tensor_trans(seg_gt)
        image = tensor_trans(image)

        Norm_ = T.Normalize((0.5), (0.5))
        image = Norm_(image)
        return image, seg_gt

    def __len__(self):
        """Returns the total number of font files."""
        return self.len
