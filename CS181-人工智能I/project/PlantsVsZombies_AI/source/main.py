from . import constants as c
from .state import mainmenu, screen, level
from .agents.env import GameState, GameRunner
from .agents.agent import Agent, RandomAgent, LocalAgent, LogicAgent


def main(args):
    # game = tool.Control()
    game = GameRunner(args.game_speed, args.level)
    state_dict = {c.MAIN_MENU: mainmenu.Menu(),
                  c.GAME_VICTORY: screen.GameVictoryScreen(),
                  c.GAME_LOSE: screen.GameLoseScreen(),
                  c.LEVEL: GameState()}

    if args.agent_type == 'random':
        agent = RandomAgent(c.RANDOM_AGENT)
    elif args.agent_type == 'manual':
        agent = Agent(c.MOUSE_AGENT)
    elif args.agent_type == 'local':
        agent = LocalAgent(c.LOACL_AGENT)
    elif args.agent_type == "logic":
        agent = LogicAgent("logicAgent")
    
    else:
        agent = RandomAgent(c.RANDOM_AGENT)

    game.setup_states(state_dict, c.LEVEL, agent)
    game.main()
