from CS_mask import cartesian_mask
import numpy as np
import itertools
import torch
from torch.utils import data as Data
from modules.utils import image2kspace, kspace2image, arbitrary_dataset_split

class DatasetReconMRI(Data.Dataset):
    def __init__(self, dataset: Data.Dataset, acc=4.0, num_center_lines=12, augment_fn=None):
        """
        :param augment_fn: perform augmentation on image data [C=2, H, W] if provided.
        """
        self.dataset = dataset

        # inferred parameter
        self.n_slices = len(dataset)

        # parameter for undersampling
        self.acc = acc
        self.num_center_lines = num_center_lines
        self.augment_fn = augment_fn

    def __getitem__(self, idx):
        im_gt = self.dataset[idx]  # [2, Nxy, Nxy] float32

        if self.augment_fn:
            im_gt = self.augment_fn(im_gt)  # [2, Nxy, Nxy] float32
        C, H, W = im_gt.shape
        und_mask = cartesian_mask(shape=(1, H, W, 1), acc=self.acc, sample_n=self.num_center_lines, centred=True
                                  ).astype(np.float32)[0, :, :, 0]  # [H, W]
        k0 = image2kspace(pseudo2real(im_gt))
        x_und, k_und = np_undersample(k0, und_mask)

        EPS = 1e-8
        x_und_abs = np.abs(x_und)
        norm_min = x_und_abs.min()
        norm_max = x_und_abs.max()
        norm_scale = norm_max - norm_min + EPS
        x_und = x_und / norm_scale
        im_gt = im_gt / norm_scale

        k_und = image2kspace(x_und)  # [H, W] Complex
        k_und = complex2pseudo(k_und)  # [C=2, H, W]
        return (
            k_und.astype(np.float32),  # [C=2, H, W]
            und_mask.astype(np.float32),  # [H, W]
            im_gt.astype(np.float32)  # [C=2, H, W]
        )

    def __len__(self):
        return self.n_slices


def get_aliased(dataset: np.array):
    mask = cartesian_mask(shape=(200, 20, 192, 192), acc=10, sample_n=10, centred=True)
    cine = dataset
    cine_kspace = image2kspace(cine)
    cine_kspace = cine_kspace *  mask
    cine_und = np.abs(kspace2image(cine_kspace))
    return mask, cine_und

def fetch_batch_sample(loader, idx):
    batch = next(itertools.islice(loader, idx, None))
    return batch

def build_loaders(dataset, train_indices, val_indices, test_indices, batch_size=10, train_augment_fn=None, num_workers=4):
    train_subset, val_subset, test_subset = arbitrary_dataset_split(
        dataset=dataset, indices_list=[
            train_indices, val_indices, test_indices]
    )

    train_dataset = DatasetReconMRI(train_subset, augment_fn=train_augment_fn)
    val_dataset = DatasetReconMRI(val_subset)
    test_dataset = DatasetReconMRI(test_subset)

    train_loader = torch.utils.data.DataLoader(train_dataset, batch_size=batch_size, num_workers=num_workers)
    val_loader = torch.utils.data.DataLoader(val_dataset,batch_size=batch_size,num_workers=num_workers)
    test_loader = torch.utils.data.DataLoader(test_dataset,batch_size=batch_size,num_workers=num_workers)

    return train_loader, val_loader, test_loader