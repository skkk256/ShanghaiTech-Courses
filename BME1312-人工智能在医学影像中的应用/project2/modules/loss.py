import torch.nn as nn


class MyBinaryCrossEntropy(object):
    def __init__(self):
        self.sigmoid = nn.Sigmoid()
        self.bce = nn.BCELoss(reduction='mean')

    def __call__(self, pred_seg, seg_gt):
        """
        :param pred_seg: [B, C=3, H, W]
        :param seg_gt: [B, C=3, H, W]
        """
        # get the probability by sigmoid. Use BCEloss.
        ######################## WRITE YOUR ANSWER BELOW ########################
        pred_seg_probs = self.sigmoid(pred_seg)
        loss = self.bce(pred_seg_probs, seg_gt)
        #########################################################################
        return loss

# class SoftDiceLossV2(_Loss):
#     __name__ = 'dice_loss'
 
#     def __init__(self, num_classes, activation='sigmoid', reduction='mean'):
#         super(SoftDiceLossV2, self).__init__()
#         self.activation = activation
#         self.num_classes = num_classes
 
#     def forward(self, y_pred, y_true):
#         class_dice = []
#         for i in range(1, self.num_classes):
#             class_dice.append(diceCoeff(y_pred[:, i:i + 1, :], y_true[:, i:i + 1, :], activation=self.activation))
#         mean_dice = sum(class_dice) / len(class_dice)
#         return 1 - mean_dice