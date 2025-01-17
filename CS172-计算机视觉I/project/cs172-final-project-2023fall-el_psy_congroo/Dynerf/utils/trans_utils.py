import torch
import numpy as np

def pose_rot(theta, phi, radius):
    c2w = torch.Tensor(np.array([[-1,0,0,0],[0,0,1,0],[0,1,0,0],[0,0,0,1]])) @ theta
    return c2w