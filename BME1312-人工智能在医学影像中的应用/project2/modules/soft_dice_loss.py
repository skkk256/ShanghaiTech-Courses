import torch.nn as nn


class SoftDiceLoss(object):
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