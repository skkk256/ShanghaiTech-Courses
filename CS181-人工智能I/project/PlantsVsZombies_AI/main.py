import pygame as pg
from source.main import main
import argparse

def config_parser():
    parser = argparse.ArgumentParser(description='plant vs zombies')
    parser.add_argument('--level', type=int, default=1, help='level of game') # 事实上僵尸出现应该是随机的
    parser.add_argument('--agent_type', type=str, default='random')
    parser.add_argument('--game_speed', type=float, default=1.0, help='running speed of game')

    return parser

if __name__=='__main__':
    parser = config_parser()
    args = parser.parse_args()

    main(args)
    pg.quit()