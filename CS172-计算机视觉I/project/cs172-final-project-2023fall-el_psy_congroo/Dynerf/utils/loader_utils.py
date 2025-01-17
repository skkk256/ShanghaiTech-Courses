import random
 import os
import numpy as np
import imageio 
import json
import cv2

import torch
from torch.utils.data.sampler import Sampler

from utils.trans_utils import pose_rot


def get_stamp_list(dataset, timestamp):
    frame_length = int(len(dataset)/len(dataset.dataset.poses))
    # print(frame_length)
    if timestamp > frame_length:
        raise IndexError("input timestamp bigger than total timestamp.")
    print("select index:",[i*frame_length+timestamp for i in range(len(dataset.dataset.poses))])
    return [dataset[i*frame_length+timestamp] for i in range(len(dataset.dataset.poses))]

class FineSampler(Sampler):
    def __init__(self, dataset):
        self.len_dataset = len(dataset) 
        self.len_pose = len(dataset.dataset.poses)
        self.frame_length = int(self.len_dataset/ self.len_pose)

        sample_list = []
        for i in range(self.frame_length):
            for j in range(4):
                idx = torch.randperm(self.len_pose) *self.frame_length + i
                now_list = []
                cnt = 0
                
                for item in idx.tolist():
                    now_list.append(item)
                    cnt+=1
                    if cnt % 2 == 0 and len(sample_list)>2:    
                        select_element = [x for x in random.sample(sample_list,2)]
                        now_list += select_element
            
            sample_list += now_list
            
        self.sample_list = sample_list
        print("one epoch containing:",len(self.sample_list))

    def __iter__(self):
        return iter(self.sample_list)
    
    def __len__(self):
        return len(self.sample_list)


trans_t = lambda t : torch.Tensor([
    [1,0,0,0],
    [0,1,0,0],
    [0,0,1,t],
    [0,0,0,1]]).float()

rot_phi = lambda phi : torch.Tensor([
    [1,0,0,0],
    [0,np.cos(phi),-np.sin(phi),0],
    [0,np.sin(phi), np.cos(phi),0],
    [0,0,0,1]]).float()

rot_theta = lambda th : torch.Tensor([
    [np.cos(th),0,-np.sin(th),0],
    [0,1,0,0],
    [np.sin(th),0, np.cos(th),0],
    [0,0,0,1]]).float()

def load_blender_data(basedir, half_res=False, testskip=1):
    splits = ['train', 'val', 'test']
    metas = {}
    for s in splits:
        with open(os.path.join(basedir, 'transforms_{}.json'.format(s)), 'r') as fp:
            metas[s] = json.load(fp)

    all_imgs = []
    all_poses = []
    all_times = []
    counts = [0]
    for s in splits:
        meta = metas[s]

        imgs = []
        poses = []
        times = []
        skip = testskip
            
        for t, frame in enumerate(meta['frames'][::skip]):
            fname = os.path.join(basedir, frame['file_path'] + '.png')
            imgs.append(imageio.imread(fname))
            poses.append(np.array(frame['transform_matrix']))
            cur_time = frame['time'] if 'time' in frame else float(t) / (len(meta['frames'][::skip])-1)
            times.append(cur_time)

        assert times[0] == 0, "Time must start at 0"

        imgs = (np.array(imgs) / 255.).astype(np.float32)  # keep all 4 channels (RGBA)
        poses = np.array(poses).astype(np.float32)
        times = np.array(times).astype(np.float32)
        counts.append(counts[-1] + imgs.shape[0])
        all_imgs.append(imgs)
        all_poses.append(poses)
        all_times.append(times)
    
    i_split = [np.arange(counts[i], counts[i+1]) for i in range(3)]
    
    imgs = np.concatenate(all_imgs, 0)
    poses = np.concatenate(all_poses, 0)
    times = np.concatenate(all_times, 0)
    
    H, W = imgs[0].shape[:2]
    camera_angle_x = float(meta['camera_angle_x'])
    focal = .5 * W / np.tan(.5 * camera_angle_x)

    if os.path.exists(os.path.join(basedir, 'transforms_{}.json'.format('render'))):
        with open(os.path.join(basedir, 'transforms_{}.json'.format('render')), 'r') as fp:
            meta = json.load(fp)
        render_poses = []
        for frame in meta['frames']:
            render_poses.append(np.array(frame['transform_matrix']))
        render_poses = np.array(render_poses).astype(np.float32)
    else:
        render_poses = torch.stack([pose_rot(angle, -30.0, 4.0) for angle in np.linspace(-180,180,40+1)[:-1]], 0)
    render_times = torch.linspace(0., 1., render_poses.shape[0])
    
    if half_res:
        H = H//2
        W = W//2
        focal = focal/2.

        imgs_half_res = np.zeros((imgs.shape[0], H, W, 4))
        for i, img in enumerate(imgs):
            imgs_half_res[i] = cv2.resize(img, (H, W), interpolation=cv2.INTER_AREA)
        imgs = imgs_half_res
        
    return imgs, poses, times, render_poses, render_times, [H, W, focal], i_split