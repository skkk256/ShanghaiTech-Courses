from typing import Sequence, List, Union

import numpy as np
from numpy.lib.stride_tricks import as_strided
import torch
from torch.utils import data as Data

from modules.utils import kspace2image, image2kspace, complex2pseudo, pseudo2real, pseudo2complex


# =============================================================================
# Utility
# =============================================================================
def arbitrary_dataset_split(dataset: Data.Dataset,
                            indices_list: Sequence[Sequence[int]]
                            ) -> List[torch.utils.data.Subset]:
    return [Data.Subset(dataset, indices) for indices in indices_list]


def datasets2loaders(datasets: Sequence[Data.Dataset],
                     *,
                     batch_size: Sequence[int] = (1, 1, 1),  # train, val, test
                     is_shuffle: Sequence[bool] = (True, False, False),  # train, val, test
                     num_workers: int = 0) -> Sequence[Data.DataLoader]:
    """
    a tool for build N-datasets into N-loaders
    """
    assert isinstance(datasets[0], Data.Dataset)
    n_loaders = len(datasets)
    assert n_loaders == len(batch_size)
    assert n_loaders == len(is_shuffle)

    loaders = []
    for i in range(n_loaders):
        loaders.append(
            Data.DataLoader(datasets[i], batch_size=batch_size[i], shuffle=is_shuffle[i], num_workers=num_workers)
        )

    return loaders


def build_loader(dataset, batch_size,
                 train_indices=np.arange(0, 112),
                 val_indices=np.arange(112, 140),
                 test_indices=np.arange(140, 200),
                 num_workers=4):
    """
    :return: train/validation/test loader
    """
    datasets = arbitrary_dataset_split(dataset, [train_indices, val_indices, test_indices])
    loaders = datasets2loaders(datasets, batch_size=(batch_size,) * 3, is_shuffle=(True, False, False),
                               num_workers=num_workers)
    return loaders


def cartesian_mask(shape, acc, sample_n=10, centred=True):
    """
    Sampling density estimated from implementation of kt FOCUSS
    shape: tuple - (Ncines, Nx, Ny, Ntime)
    shape: tuple - (Ncines, Ntime, Nx, Ny)
    acc: float - accleration factor, doesn't have to be integer 4, 8, etc..
    """

    def normal_pdf(length, sensitivity):
        return np.exp(-sensitivity * (np.arange(length) - length / 2) ** 2)
    
    
    Ncines, Ntime, Nx, Ny = shape
    N = Ncines * Ntime

    pdf_x = normal_pdf(Nx, 0.5 / (Nx / 10.) ** 2)
    lmda = Nx / (2. * acc)
    n_lines = int(Nx / acc)

    # add uniform distribution
    pdf_x += lmda * 1. / Nx

    if sample_n:
        pdf_x[Nx // 2 - sample_n // 2:Nx // 2 + sample_n // 2] = 0
        pdf_x /= np.sum(pdf_x)
        n_lines -= sample_n

    mask = np.zeros((N, Nx))
    for i in range(N):
        idx = np.random.choice(Nx, n_lines, False, pdf_x)
        mask[i, idx] = 1

    if sample_n:
        mask[:, Nx // 2 - sample_n // 2:Nx // 2 + sample_n // 2] = 1

    size = mask.itemsize
    mask = as_strided(mask, (N, Nx, Ny), (size * Nx, size, 0))

    mask = mask.reshape((Ncines, Ntime, Nx, Ny))

    if not centred:
        mask = np.fft.ifftshift(mask, axes=(2, 3))

    return mask


def np_undersample(k0, mask_centered):
    """
    input: k0 (T=20, H, W), mask_centered (T=20, H, W)
    output: x_u, k_u (20, H, W)
    """

    assert k0.shape == mask_centered.shape

    k0 = k0.astype(np.complex64)

    k_u = k0 * mask_centered
    x_u = kspace2image(k_u)

    x_u = x_u.astype(np.complex64)
    k_u = k_u.astype(np.complex64)
    return x_u, k_u


# =============================================================================
# Dataset
# =============================================================================
class LoadMRI(Data.Dataset):
    def __init__(self, path: str):
        """
        :param augment_fn: perform augmentation on image data [C=2, H, W] if provided.
        """
        data_dict = np.load(path)
        # loading dataset
        images = data_dict['dataset']  # List[ndarray]
        self.images = images

        # # preprocessing
        # images = kspace2image(kspace).astype(np.complex64)  # [1000, Nxy, Nxy] complex64
        # images = complex2pseudo(images)  # convert to pseudo-complex representation
        # self.images = images.astype(np.float32)  # [1000, 2, Nxy, Nxy] float32
        # self.viz_indices = viz_indices.astype(np.int64)  # [N,] int64

        # inferred parameter
        self.n_slices = self.images.shape[0]

    def __getitem__(self, idx):
        im_gt = self.images[idx]
        return im_gt  # [2, Nxy, Nxy] float32

    def __len__(self):
        return self.n_slices


class DatasetReconMRI(Data.Dataset):
    def __init__(self, dataset: Data.Dataset, acc=8.0, num_center_lines=12, augment_fn=None):
        """
        :param augment_fn: perform augmentation on image data [H, W, T=20] if provided.
        """
        self.dataset = dataset

        # inferred parameter
        self.n_slices = len(dataset)

        # parameter for undersampling
        self.acc = acc
        self.num_center_lines = num_center_lines
        self.augment_fn = augment_fn

    def __getitem__(self, idx):
        im_gt = self.dataset[idx]

        if self.augment_fn:
            im_gt = self.augment_fn(im_gt)  # [2, Nxy, Nxy] float32
        T, H, W = im_gt.shape
        und_mask = cartesian_mask(shape=(1, T, H, W), acc=self.acc, sample_n=self.num_center_lines, centred=True
                                  ).astype(np.float32)[0, :, :, :]  # [T, H, W]
        k0 = image2kspace(im_gt)
        x_und, k_und = np_undersample(k0, und_mask)

        # EPS = 1e-8
        # x_und_abs = np.abs(x_und)
        # norm_min = x_und_abs.min()
        # norm_max = x_und_abs.max()
        # norm_scale = norm_max - norm_min + EPS
        # x_und = x_und / norm_scale
        # im_gt = im_gt / norm_scale

        im_gt = im_gt.astype(np.complex64)
        im_gt = complex2pseudo(im_gt)
        x_und = complex2pseudo(x_und)
        
        
        return (
            x_und.astype(np.float32),  # [C=2, T, H, W]
            und_mask.astype(np.float32),  # [T, H, W]
            im_gt.astype(np.float32)  # [C=2, T, H, W]
        )

    def __len__(self):
        return self.n_slices

def build_loaders(dataset, train_indices, val_indices, test_indices, acc = 8, num_center_lines = 12, batch_size=10, train_augment_fn=None, num_workers=4):
    train_subset, val_subset, test_subset = arbitrary_dataset_split(
        dataset=dataset, indices_list=[
            train_indices, val_indices, test_indices]
    )

    train_dataset = DatasetReconMRI(train_subset, acc, num_center_lines, augment_fn=train_augment_fn)
    val_dataset = DatasetReconMRI(val_subset, acc, num_center_lines)
    test_dataset = DatasetReconMRI(test_subset, acc, num_center_lines)

    train_loader = torch.utils.data.DataLoader(train_dataset, batch_size=batch_size, num_workers=num_workers)
    val_loader = torch.utils.data.DataLoader(val_dataset,batch_size=batch_size,num_workers=num_workers)
    test_loader = torch.utils.data.DataLoader(test_dataset,batch_size=batch_size,num_workers=num_workers)

    return train_loader, val_loader, test_loader

# =============================================================================
# Test
# =============================================================================

if __name__ == '__main__':
    dataset = LoadMRI("cine.npz")
    dataset = DatasetReconMRI(dataset)
    print(len(dataset))
    k_und, und_mask, im_gt = dataset[1]
    print(f"{k_und.shape} {k_und.dtype}")
    print(f"{und_mask.shape} {und_mask.dtype}")
    print(f"{im_gt.shape} {im_gt.dtype}")
