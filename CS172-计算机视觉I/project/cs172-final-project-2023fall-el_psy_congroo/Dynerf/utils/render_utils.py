import torch

@torch.no_grad()
def get_state_at_time(pc,viewpoint_camera):    
    means3D = pc.get_xyz
    time = torch.tensor(viewpoint_camera.time).to(means3D.device).repeat(means3D.shape[0],1)
    opacity = pc._opacity
    shs = pc.get_features

    scales = pc._scaling
    rotations = pc._rotation

    means3D_final, scales_final, rotations_final, opacity_final, shs_final = pc._deformation(means3D, scales, 
                                                                 rotations, opacity, shs,
                                                                 time)
    scales_final = pc.scaling_activation(scales_final)
    rotations_final = pc.rotation_activation(rotations_final)
    opacity = pc.opacity_activation(opacity_final)
    return means3D_final, scales_final, rotations_final, opacity, shs