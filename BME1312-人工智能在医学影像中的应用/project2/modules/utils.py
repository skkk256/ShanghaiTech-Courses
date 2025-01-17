import math
import io

import numpy as np
import torch
import torchvision.utils
from matplotlib import pyplot as plt
from matplotlib.colors import Normalize
from mpl_toolkits.axes_grid1 import make_axes_locatable
from skimage.color import rgb2gray
from skimage.metrics import structural_similarity

######################################################


def mask_to_onehot(mask, palette):
    """
    Converts a segmentation mask (H, W, C) to (H, W, K) where the last dim is a one
    hot encoding vector, C is usually 1 or 3, and K is the number of class.
    """
    semantic_map = []
    for colour in palette:
        equality = np.equal(mask, colour)
        class_map = np.all(equality, axis=-1)
        semantic_map.append(class_map)
    semantic_map = np.stack(semantic_map, axis=-1).astype(np.float32)
    return semantic_map


def onehot_to_mask(mask, palette):
    """
    Converts a mask (H, W, K) to (C, H, W)
    """
    x = np.argmax(mask, axis=0)
    colour_codes = np.array(palette)
    x = np.uint8(colour_codes[x.astype(np.uint8)])
    return x

######################################################


def plot_loss(loss):
    plt.figure()
    plt.plot(loss)
    plt.show()
    plt.close('all')


def imgshow(im, cmap=None, rgb_axis=None, dpi=100, figsize=(6.4, 4.8)):
    if isinstance(im, torch.Tensor):
        im = im.to('cpu').detach().cpu().numpy()
    if rgb_axis is not None:
        im = np.moveaxis(im, rgb_axis, -1)
        im = rgb2gray(im)

    plt.figure(dpi=dpi, figsize=figsize)
    norm_obj = Normalize(vmin=im.min(), vmax=im.max())
    plt.imshow(im, norm=norm_obj, cmap=cmap)
    plt.colorbar()
    plt.show()
    plt.close('all')


def imsshow(imgs, titles=None, num_col=5, dpi=100, cmap=None, is_colorbar=False, is_ticks=False, ylim : list=[]):
    '''
    assume imgs's shape is (Nslice, Nx, Ny)
    ylim is [y1, y2]] or default []
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
            cax = fig.add_axes(
                [ax.get_position().x1 + 0.01, ax.get_position().y0, 0.01, ax.get_position().height])
            plt.colorbar(im, cax=cax)
        if not is_ticks:
            ax.set_xticks([])
            ax.set_yticks([])
        if ylim != []:
            plt.ylim(ylim[0], ylim[1])
    plt.show()
    plt.close('all')


def image_mask_overlay(image, mask) -> np.ndarray:
    """
    :param image: [H, W] float(0~1) or uint8(0~255)
    :param mask: [H, W] int64
    :return: [H, W, C]
    """

    def _fig2numpy(fig, dpi) -> np.ndarray:
        """
        Convert matplotlib figure to numpy array
        """
        io_buf = io.BytesIO()
        fig.savefig(io_buf, format='raw', dpi=dpi)
        io_buf.seek(0)
        img_arr = np.reshape(np.frombuffer(io_buf.getvalue(), dtype=np.uint8),
                             newshape=(int(fig.bbox.bounds[3]), int(fig.bbox.bounds[2]), -1))
        io_buf.close()
        return img_arr

    H, W = image.shape
    dpi = H
    # dpi = dpi * factor
    fig = plt.figure(figsize=(math.ceil(H / dpi), math.ceil(W / dpi)), dpi=dpi)
    plt.xticks([])
    plt.yticks([])
    ax = fig.subplots(1, 1)
    fig.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=0, hspace=0)

    ax.imshow(image, cmap='gray', interpolation='nearest')
    ax.imshow(mask, cmap='jet', alpha=0.5)
    ax.axis('off')
    im = _fig2numpy(fig, dpi=dpi)
    plt.close(fig)
    return im


def make_grid_and_show(ims, nrow=5, cmap=None):
    if isinstance(ims, np.ndarray):
        ims = torch.from_numpy(ims)

    B, C, H, W = ims.shape
    grid_im = torchvision.utils.make_grid(ims, nrow=nrow)
    fig_h, fig_w = nrow * 2 + 1, (B / nrow) + 1
    imgshow(grid_im, cmap=cmap, rgb_axis=0, dpi=200, figsize=(fig_h, fig_w))


def int2preetyStr(num: int):
    s = str(num)
    remain_len = len(s)
    while remain_len - 3 > 0:
        s = s[:remain_len - 3] + ',' + s[remain_len - 3:]
        remain_len -= 3
    return s


def compute_num_params(module, is_trace=False):
    print(int2preetyStr(sum([p.numel() for p in module.parameters()])))
    if is_trace:
        for item in [f"[{int2preetyStr(info[1].numel())}] {info[0]}:{tuple(info[1].shape)}"
                     for info in module.named_parameters()]:
            print(item)


def tonp(x):
    if isinstance(x, torch.Tensor):
        return x.detach().cpu()
    else:
        return x


def pseudo2real(x):
    """
    :param x: [..., C=2, H, W]
    :return: [..., H, W]
    """
    return (x[..., 0, :, :] ** 2 + x[..., 1, :, :] ** 2) ** 0.5


def complex2pseudo(x):
    """
    :param x: [..., H, W] Complex
    :return: [...., C=2, H, W]
    """
    if isinstance(x, np.ndarray):
        return np.stack([x.real, x.imag], axis=-3)
    elif isinstance(x, torch.Tensor):
        return torch.stack([x.real, x.imag], dim=-3)
    else:
        raise RuntimeError("Unsupported type.")


def pseudo2complex(x):
    """
    :param x:  [..., C=2, H, W]
    :return: [..., H, W] Complex
    """
    return x[..., 0, :, :] + x[..., 1, :, :] * 1j


# ================================
# Preprocessing
# ================================
def minmax_normalize(x, eps=1e-8):
    min = x.min()
    max = x.max()
    return (x - min) / (max - min + eps)


# ================================
# kspace and image domain transform
# reference: [ismrmrd-python-tools/transform.py at master · ismrmrd/ismrmrd-python-tools · GitHub](https://github.com/ismrmrd/ismrmrd-python-tools/blob/master/ismrmrdtools/transform.py)
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
        max_intensity = np.max(target_im).item(
        ) if peak == 'max' else max_intensity
        psnr = 20 * math.log10(max_intensity) - 10 * \
            np.log10(compute_mse(reconstructed_im, target_im) + eps)

    elif isinstance(target_im, torch.Tensor):
        max_intensity = 255 if target_im.dtype == torch.uint8 else 1.0
        max_intensity = torch.max(target_im).item(
        ) if peak == 'max' else max_intensity
        psnr = 20 * math.log10(max_intensity) - 10 * \
            torch.log10(compute_mse(reconstructed_im, target_im) + eps)

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

    ssim_value = structural_similarity(target_im, reconstructed_im,
                                       gaussian_weights=True, sigma=1.5, use_sample_covariance=False)

    return ssim_value
