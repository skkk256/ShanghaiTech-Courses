from trainer import Trainer
from options import MonodepthOptions

args = MonodepthOptions()
args = args.parser.parse_args()

# args.data_path = "/data/shengkuan/CS172/cs172-assignment2-2023fall-skkk256/data/kitti_data"
# args.log_dir = "/data/shengkuan/CS172/cs172-assignment2-2023fall-skkk256/log"
# args.model_name = "test"
# args.png = True
# # args.batch_size = 1
# args.num_workers = 0


if __name__ == "__main__":
    trainer = Trainer(args)
    trainer.train()