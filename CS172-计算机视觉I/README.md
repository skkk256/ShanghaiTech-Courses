
# 计算机视觉I

**学分**: 4
**授课老师**: 高盛华


## 课程评价

碰上第一年改革，形式非常激进、内容也很前沿的cv课，抛弃了传统cv课程中所有图像处理、各种kernel、各种task, 只关注在三维视觉领域。

学的时候很折磨，但是现在看还是很有意义的

非常可惜高盛华(上科大)已经变成高盛华(香港大学)了

## 详细信息

从相机模型、内外参矩阵开始讲起，讲到多视角、单视角(双目/单目)的相机标定、Epipolar、homography等等，最后讲SDF、nerf和deep learning

三次作业，工作量还比较大其实，但滑稽的是三次作业加起来占比 10%，和签到一个分

- 第一次是 ResNet 和 MCNN 做人群计数
- 第二次是 Self-Supervised Monocular Depth Prediction
  1. 实现Geometry to the rescue 
  2. 处理遮挡物体导致的深度估计误差
  3. 如何利用几何约束实现深度估计性能提高
- 第三次是 NeRF
  1. 实现NeRF accelerate method (TensoRF and NGP)
  2. 构建3个自己拍摄的场景的数据集 (refer to nerfstudio)，并在实现的NeRF accelerate method 测试性能
  3. Bonus: 思考如何将 NeRF accelerate method (TensoRF and NGP) 合理结合，进一步提升性能

Project 拿3d gaussian + dnerf 糊了一下；从给定题题目中选，
，包括
1. Novel View Synthesis for Dynamic Scene
2. Animatable Human Avatar
3. Indoor scene reconstruction
4. Disentangling Object Motion for Self-supervised Depth Estimation



CS 极少数的闭卷考试，不算很难，但是占比巨大。考公式推导，都是大题，包括相机矩阵各种标定、Epipolar、NeRF