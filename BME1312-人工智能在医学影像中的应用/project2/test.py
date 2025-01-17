import torch
from torch.utils.tensorboard import SummaryWriter

# 初始化TensorBoard的输出路径
writer = SummaryWriter('logs')

# 定义训练和验证数据集
train_dataset = ...
val_dataset = ...

# 定义训练和验证数据集的data loader
train_loader = torch.utils.data.DataLoader(train_dataset, batch_size=32, shuffle=True)
val_loader = torch.utils.data.DataLoader(val_dataset, batch_size=32, shuffle=True)

# 定义模型和优化器
model = ...
optimizer = ...

# 开始训练
for epoch in range(10):
    # 训练
    model.train()
    train_loss = 0
    for batch_idx, (data, target) in enumerate(train_loader):
        optimizer.zero_grad()
        output = model(data)
        loss = ...
        loss.backward()
        optimizer.step()
        train_loss += loss.item()
        
        # 将训练loss添加到TensorBoard
        writer.add_scalar('Train Loss', train_loss, epoch*len(train_loader)+batch_idx)
        
    # 验证
    model.eval()
    val_loss = 0
    with torch.no_grad():
        for batch_idx, (data, target) in enumerate(val_loader):
            output = model(data)
            loss = ...
            val_loss += loss.item()

            validation_image = ...
            # 将验证图片添加到TensorBoard
            writer.add_image('Validation Images', validation_image, epoch*len(val_loader)+batch_idx)

    # 将验证loss添加到TensorBoard
    writer.add_scalar('Validation Loss', val_loss, epoch)
  
# 关闭SummaryWriter
writer.close()
