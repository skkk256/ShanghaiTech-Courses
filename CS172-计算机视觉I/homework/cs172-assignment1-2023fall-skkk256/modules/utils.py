import cv2
import numpy as np
import scipy


def get_density_map_gaussian(im, points, adaptive_mode=False, fixed_value=15, fixed_values=None):
    density_map = np.zeros(im.shape[:2], dtype=np.float32)
    h, w = density_map.shape[:2]
    num_gt = np.squeeze(points).shape[0]
    if num_gt == 0:
        return density_map

    if adaptive_mode == True:
        fixed_values = None
        leafsize = 2048
        tree = scipy.spatial.KDTree(points.copy(), leafsize=leafsize)
        distances, locations = tree.query(points, k=4)
    for idx, p in enumerate(points):
        p = np.round(p).astype(int)
        p[0], p[1] = min(h-1, p[1]), min(w-1, p[0])
        if num_gt > 1:
            if adaptive_mode == 1:
                sigma = int(np.sum(distances[idx][1:4]) * 0.1)
            elif adaptive_mode == 0:
                sigma = fixed_value
        else:
            sigma = fixed_value
        sigma = max(1, sigma)
        gaussian_radius_no_detection = sigma * 3
        gaussian_radius = gaussian_radius_no_detection

        if fixed_values is not None:
            grid_y, grid_x = int(p[0]//(h/3)), int(p[1]//(w/3))
            grid_idx = grid_y * 3 + grid_x
            gaussian_radius = fixed_values[grid_idx] if fixed_values[grid_idx] else gaussian_radius_no_detection
        gaussian_map = np.multiply(
            cv2.getGaussianKernel(gaussian_radius*2+1, sigma),
            cv2.getGaussianKernel(gaussian_radius*2+1, sigma).T
        )
        gaussian_map[gaussian_map < 0.0003] = 0
        if np.sum(gaussian_map):
            gaussian_map = gaussian_map / np.sum(gaussian_map)
        x_left, x_right, y_up, y_down = 0, gaussian_map.shape[1], 0, gaussian_map.shape[0]
        # print(x_left, x_right)
        # cut the gaussian kernel
        if p[1] < gaussian_radius:
            x_left = gaussian_radius - p[1]
        if p[0] < gaussian_radius:
            y_up = gaussian_radius - p[0]
        if p[1] + gaussian_radius >= w:
            x_right = gaussian_map.shape[1] - (gaussian_radius + p[1] - w) - 1
        if p[0] + gaussian_radius >= h:
            y_down = gaussian_map.shape[0] - (gaussian_radius + p[0] - h) - 1

        density_map[
            max(0, p[0]-gaussian_radius):min(density_map.shape[0], p[0]+gaussian_radius+1),
            max(0, p[1]-gaussian_radius):min(density_map.shape[1], p[1]+gaussian_radius+1)
        ] += gaussian_map[y_up:y_down, x_left:x_right]
    # density_map[density_map < 0.0003] = 0
    density_map = density_map / (np.sum(density_map / num_gt))
    return density_map
