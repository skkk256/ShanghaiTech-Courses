import numpy as np
import torch
import math
from torch.utils import data as Data
from typing import Sequence, List, Union
from skimage.metrics import structural_similarity
from matplotlib import pyplot as plt

# ================================
# Visualization
# ================================

def imsshow(imgs, titles=None, num_col=5, dpi=100, cmap=None, is_colorbar=False, is_ticks=False):
    '''
    assume imgs's shape is (Nslice, Nx, Ny)
    '''
    num_imgs = len(imgs)
    num_row = math.ceil(num_imgs / num_col)
    fig_width = num_col * 3
    if is_colorbar:
        fig_width += num_col * 1.5
    fig_height = num_row * 3
    fig = plt.figure(dpi=dpi, figsize=(fig_width, fig_height))
    for i in range(num_imgs):
        ax = plt.subplot(num_row, num_col, i + 1)
        im = ax.imshow(imgs[i], cmap=cmap)
        if titles:
            plt.title(titles[i])
        if is_colorbar:
            cax = fig.add_axes([ax.get_position().x1 + 0.01, ax.get_position().y0, 0.01, ax.get_position().height])
            plt.colorbar(im, cax=cax)
        if not is_ticks:
            ax.set_xticks([])
            ax.set_yticks([])
    plt.show()
    plt.close('all')

# ================================
# Data Converter
# ================================

def image2kspace(x):
    if isinstance(x, np.ndarray):
        x = np.fft.ifftshift(x, axes=(-2, -1))
        x = np.fft.fft2(x)
        x = np.fft.fftshift(x, axes=(-2, -1))
        return x
    elif isinstance(x, torch.Tensor):
        x = torch.fft.ifftshift(x, dim=(-2, -1))
        x = torch.fft.fft2(x)
        x = torch.fft.fftshift(x, dim=(-2, -1))
        return x
    else:
        raise RuntimeError("Unsupported type.")

def kspace2image(x):
    if isinstance(x, np.ndarray):
        x = np.fft.ifftshift(x, axes=(-2, -1))
        x = np.fft.ifft2(x)
        x = np.fft.fftshift(x, axes=(-2, -1))
        return x
    elif isinstance(x, torch.Tensor):
        x = torch.fft.ifftshift(x, dim=(-2, -1))
        x = torch.fft.ifft2(x)
        x = torch.fft.fftshift(x, dim=(-2, -1))
        return x
    else:
        raise RuntimeError("Unsupported type.")

def pseudo2real(x):
    """
    :param x: [..., C=2, T, H, W]
    :return: [..., T, H, W]
    """
    return (x[..., 0, :, :, :] ** 2 + x[..., 1, :, :, :] ** 2) ** 0.5


def complex2pseudo(x):
    """
    :param x: [...T, H, W] Complex
    :return: [...., C=2, T, H, W]
    """
    if isinstance(x, np.ndarray):
        return np.stack([x.real, x.imag], axis=-4)
    elif isinstance(x, torch.Tensor):
        return torch.stack([x.real, x.imag], dim=-4)
    else:
        raise RuntimeError("Unsupported type.")

def pseudo2complex(x):
    """
    :param x:  [..., C=2, T, H, W]
    :return: [..., T, H, W] Complex
    """
    return x[..., 0, :, :, :] + x[..., 1, :, :, :] * 1j

# def arbitrary_dataset_split(dataset: Data.Dataset,
#                             indices_list: Sequence[Sequence[int]]
#                             ) -> List[torch.utils.data.Subset]:
#     return [Data.Subset(dataset, indices) for indices in indices_list]


# ================================
# Preprocessing
# ================================
def minmax_normalize(x, eps=1e-8):
    min = x.min()
    max = x.max()
    return (x - min) / (max - min + eps)


# ======================================
# Metrics
# ======================================
def compute_mse(x, y):
    """
    REQUIREMENT: `x` and `y` can be any shape, but their shape have to be same
    """
    assert x.dtype == y.dtype and x.shape == y.shape, \
        'x and y is not compatible to compute MSE metric'

    if isinstance(x, np.ndarray):
        mse = np.mean(np.abs(x - y) ** 2)

    elif isinstance(x, torch.Tensor):
        mse = torch.mean(torch.abs(x - y) ** 2)

    else:
        raise RuntimeError(
            'Unsupported object type'
        )
    return mse


def compute_psnr(reconstructed_im, target_im, peak='normalized', is_minmax=False):
    '''
    Image must be of either Integer [0, 255] or Float value [0,1]
    :param peak: 'max' or 'normalize', max_intensity will be the maximum value of target_im if peek == 'max.
          when peek is 'normalized', max_intensity will be the maximum value depend on data representation (in this
          case, we assume your input should be normalized to [0,1])
    REQUIREMENT: `x` and `y` can be any shape, but their shape have to be same
    '''
    assert target_im.dtype == reconstructed_im.dtype and target_im.shape == reconstructed_im.shape, \
        'target_im and reconstructed_im is not compatible to compute PSNR metric'
    assert peak in {'max', 'normalized'}, \
        'peak mode is not supported'

    eps = 1e-8  # to avoid math error in log(x) when x=0

    if is_minmax:
        reconstructed_im = minmax_normalize(reconstructed_im, eps)
        target_im = minmax_normalize(target_im, eps)

    if isinstance(target_im, np.ndarray):
        max_intensity = 255 if target_im.dtype == np.uint8 else 1.0
        max_intensity = np.max(target_im).item() if peak == 'max' else max_intensity
        psnr = 20 * math.log10(max_intensity) - 10 * np.log10(compute_mse(reconstructed_im, target_im) + eps)

    elif isinstance(target_im, torch.Tensor):
        max_intensity = 255 if target_im.dtype == torch.uint8 else 1.0
        max_intensity = torch.max(target_im).item() if peak == 'max' else max_intensity
        psnr = 20 * math.log10(max_intensity) - 10 * torch.log10(compute_mse(reconstructed_im, target_im) + eps)

    else:
        raise RuntimeError(
            'Unsupported object type'
        )
    return psnr


def compute_ssim(reconstructed_im, target_im, is_minmax=False):
    """
    Compute structural similarity index between two batches using skimage library,
    which only accept 2D-image input. We have to specify where is image's axes.

    WARNING: this method using skimage's implementation, DOES NOT SUPPORT GRADIENT
    """
    assert target_im.dtype == reconstructed_im.dtype and target_im.shape == reconstructed_im.shape, \
        'target_im and reconstructed_im is not compatible to compute SSIM metric'

    if isinstance(target_im, np.ndarray):
        pass
    elif isinstance(target_im, torch.Tensor):
        target_im = target_im.detach().to('cpu').numpy()
        reconstructed_im = reconstructed_im.detach().to('cpu').numpy()
    else:
        raise RuntimeError(
            'Unsupported object type'
        )
    
    eps = 1e-8  # to avoid math error in log(x) when x=0

    if is_minmax:
        reconstructed_im = minmax_normalize(reconstructed_im, eps)
        target_im = minmax_normalize(target_im, eps)
    
    ssim_value = structural_similarity(target_im, reconstructed_im, \
        gaussian_weights=True, sigma=1.5, data_range = 1, use_sample_covariance=False)

    return ssim_value