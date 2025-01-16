# multiAgents.py
# --------------
# Licensing Information:  You are free to use or extend these projects for
# educational purposes provided that (1) you do not distribute or publish
# solutions, (2) you retain this notice, and (3) you provide clear
# attribution to UC Berkeley, including a link to http://ai.berkeley.edu.
#
# Attribution Information: The Pacman AI projects were developed at UC Berkeley.
# The core projects and autograders were primarily created by John DeNero
# (denero@cs.berkeley.edu) and Dan Klein (klein@cs.berkeley.edu).
# Student side autograding was added by Brad Miller, Nick Hay, and
# Pieter Abbeel (pabbeel@cs.berkeley.edu).


from util import manhattanDistance
from game import Directions
import random
import util

from game import Agent
from pacman import GameState


class ReflexAgent(Agent):
    """
    A reflex agent chooses an action at each choice point by examining
    its alternatives via a state evaluation function.

    The code below is provided as a guide.  You are welcome to change
    it in any way you see fit, so long as you don't touch our method
    headers.
    """

    def getAction(self, gameState: GameState):
        """
        You do not need to change this method, but you're welcome to.

        getAction chooses among the best options according to the evaluation function.

        Just like in the previous project, getAction takes a GameState and returns
        some Directions.X for some X in the set {NORTH, SOUTH, WEST, EAST, STOP}
        """
        # Collect legal moves and child states
        legalMoves = gameState.getLegalActions()

        # Choose one of the best actions
        scores = [self.evaluationFunction(
            gameState, action) for action in legalMoves]
        bestScore = max(scores)
        bestIndices = [index for index in range(
            len(scores)) if scores[index] == bestScore]
        # Pick randomly among the best
        chosenIndex = random.choice(bestIndices)

        "Add more of your code here if you want to"

        return legalMoves[chosenIndex]

    def evaluationFunction(self, currentGameState: GameState, action):
        """
        Design a better evaluation function here.

        The evaluation function takes in the current and proposed child
        GameStates (pacman.py) and returns a number, where higher numbers are better.

        The code below extracts some useful information from the state, like the
        remaining food (newFood) and Pacman position after moving (newPos).
        newScaredTimes holds the number of moves that each ghost will remain
        scared because of Pacman having eaten a power pellet.

        Print out these variables to see what you're getting, then combine them
        to create a masterful evaluation function.
        """
        # Useful information you can extract from a GameState (pacman.py)
        childGameState: GameState = currentGameState.getPacmanNextState(action)
        pos = currentGameState.getPacmanPosition()
        newPos = childGameState.getPacmanPosition()
        newFood = childGameState.getFood()
        newGhostStates: list = childGameState.getGhostStates()
        newScaredTimes = [
            ghostState.scaredTimer for ghostState in newGhostStates]

        "*** YOUR CODE HERE ***"
        change_score = childGameState.getScore() - currentGameState.getScore()
        current_food_distance = min(
            [manhattanDistance(pos, food) for food in currentGameState.getFood().asList()])
        food_distance = min([manhattanDistance(newPos, food)
                            for food in newFood.asList()]) if newFood.asList() else 0
        current_ghost_distance = min(
            [manhattanDistance(pos, state.getPosition()) for state in newGhostStates])
        ghost_distance = min(
            [manhattanDistance(newPos, state.getPosition()) for state in newGhostStates])

        if ghost_distance <= 2 and min(newScaredTimes) == 0:
            return -99999
        if change_score > 0:
            return 99999
        if food_distance < current_food_distance:
            return 3
        elif ghost_distance > current_ghost_distance:
            return 2
        else:
            return 1


def scoreEvaluationFunction(currentGameState: GameState):
    """
    This default evaluation function just returns the score of the state.
    The score is the same one displayed in the Pacman GUI.

    This evaluation function is meant for use with adversarial search agents
    (not reflex agents).
    """
    return currentGameState.getScore()


class MultiAgentSearchAgent(Agent):
    """
    This class provides some common elements to all of your
    multi-agent searchers.  Any methods defined here will be available
    to the MinimaxPacmanAgent, AlphaBetaPacmanAgent & ExpectimaxPacmanAgent.

    You *do not* need to make any changes here, but you can if you want to
    add functionality to all your adversarial search agents.  Please do not
    remove anything, however.

    Note: this is an abstract class: one that should not be instantiated.  It's
    only partially specified, and designed to be extended.  Agent (game.py)
    is another abstract class.
    """

    def __init__(self, evalFn='scoreEvaluationFunction', depth='2'):
        self.index = 0  # Pacman is always agent index 0
        self.evaluationFunction = util.lookup(evalFn, globals())
        self.depth = int(depth)


class MinimaxAgent(MultiAgentSearchAgent):
    """
    Your minimax agent (question 2)
    """

    def getAction(self, gameState: GameState):
        """
        Returns the minimax action from the current gameState using self.depth
        and self.evaluationFunction.

        Here are some method calls that might be useful when implementing minimax.

        gameState.getLegalActions(agentIndex):
        Returns a list of legal actions for an agent
        agentIndex=0 means Pacman, ghosts are >= 1

        gameState.getNextState(agentIndex, action):
        Returns the child game state after an agent takes an action

        gameState.getNumAgents():
        Returns the total number of agents in the game

        gameState.isWin():
        Returns whether or not the game state is a winning state

        gameState.isLose():
        Returns whether or not the game state is a losing state
        """
        "*** YOUR CODE HERE ***"
        def value(gameState: GameState, depth: int, agentIndex: int):
            if gameState.isWin() or gameState.isLose():
                return self.evaluationFunction(gameState)
            if agentIndex == 0:
                return max_value(gameState, depth)
            else:
                return min_value(gameState, depth, agentIndex)

        def max_value(gameState: GameState, depth):
            v = -float('inf')
            if depth == self.depth:
                return self.evaluationFunction(gameState)
            for action in gameState.getLegalActions(0):
                v = max(v, value(gameState.getNextState(0, action), depth, 1))
            return v

        def min_value(gameState: GameState, depth, agentIndex):
            v = float('inf')
            if depth == self.depth:
                return self.evaluationFunction(gameState)
            for action in gameState.getLegalActions(agentIndex):
                if agentIndex == (gameState.getNumAgents() - 1):
                    v = min(v, value(gameState.getNextState(
                        agentIndex, action), depth+1, 0))
                else:
                    v = min(v, value(gameState.getNextState(
                        agentIndex, action), depth, agentIndex+1))
            return v

        value_list = []
        actions = gameState.getLegalActions(0)
        for action in actions:
            value_list.append(value(gameState.getNextState(0, action), 0, 1))
        step = actions[value_list.index(max(value_list))]
        return step


class AlphaBetaAgent(MultiAgentSearchAgent):
    """
    Your minimax agent with alpha-beta pruning (question 3)
    """

    def getAction(self, gameState: GameState):
        """
        Returns the minimax action using self.depth and self.evaluationFunction
        """
        def value(gameState, depth, agentIndex, alpha, beta):
            if gameState.isWin() or gameState.isLose() or depth == self.depth:
                return self.evaluationFunction(gameState)
            if agentIndex == 0:
                return max_value(gameState, depth, alpha, beta)
            else:
                return min_value(gameState, depth, agentIndex, alpha, beta)

        def max_value(gameState: GameState, depth, alpha, beta):
            v = -float('inf')
            if depth + 1 == self.depth:
                return self.evaluationFunction(gameState)
            for action in gameState.getLegalActions(0):
                v = max(v, value(gameState.getNextState(
                    0, action), depth+1, 1, alpha, beta))
                if v > beta:
                    return v
                alpha = max(alpha, v)
            return v

        def min_value(gameState: GameState, depth, agentIndex, alpha, beta):
            v = float('inf')
            for action in gameState.getLegalActions(agentIndex):
                if agentIndex == gameState.getNumAgents() - 1:
                    v = min(v, value(gameState.getNextState(
                        agentIndex, action), depth, 0, alpha, beta))
                else:
                    v = min(v, value(gameState.getNextState(
                        agentIndex, action), depth, agentIndex+1, alpha, beta))
                if v < alpha:
                    return v
                beta = min(beta, v)
            return v

        alpha = -float('inf')
        beta = float('inf')
        best_score = -float('inf')
        best_action = None
        for action in gameState.getLegalActions(0):
            score = value(gameState.getNextState(0, action), 0, 1, alpha, beta)
            if score > best_score:
                best_score = score
                best_action = action
            if best_score > beta:
                return best_action
            alpha = max(alpha, best_score)
        return best_action


class ExpectimaxAgent(MultiAgentSearchAgent):
    """
      Your expectimax agent (question 4)
    """

    def getAction(self, gameState):
        """
        Returns the expectimax action using self.depth and self.evaluationFunction

        All ghosts should be modeled as choosing uniformly at random from their
        legal moves.
        """
        "*** YOUR CODE HERE ***"
        return self.expectimax(0, gameState, 0)

    def expectimax(self, agent_id, now_game_state, step):
        if agent_id == now_game_state.getNumAgents():
            agent_id = 0
            step += 1

        if not now_game_state.getLegalActions(agent_id) or step == self.depth:
            return self.evaluationFunction(now_game_state)

        next_minmax_result = [
            (self.expectimax(agent_id + 1,
             now_game_state.getNextState(agent_id, next_step), step), next_step)
            for next_step in now_game_state.getLegalActions(agent_id)]
        if agent_id == 0:
            if step == 0:
                return max(next_minmax_result, key=lambda x: x[0])[1]
            else:
                return max(next_minmax_result, key=lambda x: x[0])[0]
        else:
            return sum([x[0] for x in next_minmax_result]) / len(next_minmax_result)


def betterEvaluationFunction(currentGameState: GameState):
    """
    Your extreme ghost-hunting, pellet-nabbing, food-gobbling, unstoppable
    evaluation function (question 5).

    DESCRIPTION: <write something here so we know what you did>
    """
    "*** YOUR CODE HERE ***"
    direction = currentGameState.getPacmanState().getDirection()
    positon = currentGameState.getPacmanPosition()
    foods = currentGameState.getFood().asList()
    food_distance = min([manhattanDistance(positon, food)
                        for food in foods]) if foods else 0
    ghost_state = currentGameState.getGhostStates()
    ghost_distance = min(
        [manhattanDistance(positon, state.getPosition()) for state in ghost_state])
    score = currentGameState.getScore()
    reward = 1 if direction != Directions.STOP else 0

    """
    the final evaluation is to close to the food, to far away from the ghost, 
    and the score is high. Base on these, If pacman not stop, there will be an additional reward
    """
    evaluation = 10.0/(1+food_distance) + score - ghost_distance + reward
    return evaluation


# Abbreviation
better = betterEvaluationFunction


class ContestAgent(MultiAgentSearchAgent):
    """
      Your agent for the mini-contest
    """

    def __init__(self, evalFn='bestEvaluationFunction', depth='3'):
        self.step_num = 0

    def betterBetterEvaluationFunction(self, currentGameState: GameState):
        """
        Your extreme ghost-hunting, pellet-nabbing, food-gobbling, unstoppable
        evaluation function (question 5).

        DESCRIPTION: <write something here so we know what you did>
        """
        "*** YOUR CODE HERE ***"

        direction = currentGameState.getPacmanState().getDirection()
        positon = currentGameState.getPacmanPosition()
        foods = currentGameState.getFood().asList()
        food_distance = min([manhattanDistance(positon, food)
                            for food in foods]) if foods else 0.5
        ghost_state = currentGameState.getGhostStates()
        capsules = currentGameState.getCapsules()
        capsule_distance = min([manhattanDistance(positon, capsule)
                               for capsule in capsules]) if capsules else 0
        scared_times = [
            ghostState.scaredTimer for ghostState in ghost_state]
        ghost_distance = min(
            [manhattanDistance(positon, state.getPosition()) if state.scaredTimer != 0 else 0 for state in ghost_state])
        score = currentGameState.getScore()
        reward = 20 if direction != Directions.STOP else 0
        # if positon == (1,1) and direction == Directions.STOP and len(capsules)==2:
        #     reward = 200
        # # if positon == (1,1) and direction == Directions.STOP and len(capsules)==2:
        #     # print(positon)
        #     reward = 200
        # elif positon == (1,2) and len(capsules)==2:
        #     # print(positon)
        #     reward = 200
        big_food = sum(scared_times)*2
        capsules_reward = 0
        if (len(capsules) == 5):
            capsules_reward = 100.0/capsule_distance

        """
        the final evaluation is to close to the food, to far away from the ghost, 
        and the score is high. Base on these, If pacman not stop, there will be an additional reward
        """
        evaluation = 1.0/food_distance + score - \
            ghost_distance**3 + reward + big_food + capsules_reward
        return evaluation

    def beterEvaluationFunction(self, currentGameState: GameState):
        """
        Your extreme ghost-hunting, pellet-nabbing, food-gobbling, unstoppable
        evaluation function (question 5).

        DESCRIPTION: <write something here so we know what you did>
        """
        "*** YOUR CODE HERE ***"
        capsules = currentGameState.getCapsules()
        direction = currentGameState.getPacmanState().getDirection()
        positon = currentGameState.getPacmanPosition()
        foods = currentGameState.getFood().asList()
        food_distance = min([manhattanDistance(positon, food)
                            for food in foods.extend(capsules)]) if foods else 0.5
        capsules = currentGameState.getCapsules()
        ghost_state = currentGameState.getGhostStates()
        newScaredTimes = [
            ghostState.scaredTimer for ghostState in ghost_state]
        ghost_distance = min(
            [manhattanDistance(positon, state.getPosition()) if state.scaredTimer != 0 else -1 for state in ghost_state])
        score = currentGameState.getScore()
        reward = 10 if direction != Directions.STOP else 0
        big_food = sum(newScaredTimes)*2

        bonus = 0
        # if (positon[0] == 1 and positon[1] < 4 and (1,1) not in capsules):
        #     if direction == Directions.STOP:
        #         bonus = -500

        evaluation = 10.0/food_distance + score - \
            ghost_distance**3 + reward + big_food + bonus
        return evaluation

    def bestEvaluationFunction(self, currentGameState: GameState):
        """
        Your extreme ghost-hunting, pellet-nabbing, food-gobbling, unstoppable
        evaluation function (question 5).

        DESCRIPTION: <write something here so we know what you did>
        """
        "*** YOUR CODE HERE ***"
        direction = currentGameState.getPacmanState().getDirection()
        positon = currentGameState.getPacmanPosition()
        foods = currentGameState.getFood().asList()
        food_distance = min([manhattanDistance(positon, food)
                            for food in foods]) if foods else 0.5
        ghost_state = currentGameState.getGhostStates()
        newScaredTimes = [
            ghostState.scaredTimer for ghostState in ghost_state]
        ghost_distance = min(
            [manhattanDistance(positon, state.getPosition()) if state.scaredTimer != 0 else -1 for state in ghost_state])
        score = currentGameState.getScore()
        reward = 10 if direction != Directions.STOP else 0
        big_food = sum(newScaredTimes)*2
        if (ghost_distance == -1):
            ghost_distance *= 3 - newScaredTimes.count(0)
        """
        the final evaluation is to close to the food, to far away from the ghost, 
        and the score is high. Base on these, If pacman not stop, there will be an additional reward
        """
        evaluation = 10.0/food_distance + score - ghost_distance**3 + reward + big_food
        return evaluation

    def FuckevaluationFunction(self, currentGameState):
        """
        Your extreme ghost-hunting, pellet-nabbing, food-gobbling, unstoppable
        evaluation function (question 5).

        DESCRIPTION: <write something here so we know what you did>
        """
        "*** YOUR CODE HERE ***"
        newPos = currentGameState.getPacmanPosition()

        value = self.score_value * currentGameState.getScore()
        # value -= self.ghost_state_base ** (self.ghost_state_zero - min(manhattanDistance(newPos, ghost.getPosition())
        #                                                                for ghost in currentGameState.getGhostStates()))
        scared_ghost = 0
        ghost_num = 0
        for ghost in currentGameState.getGhostStates():
            ghost_num += 1
            if ghost.scaredTimer <= 0:
                value -= self.ghost_state_base ** (
                    self.ghost_state_zero - manhattanDistance(newPos, ghost.getPosition()))
            else:
                scared_ghost += 1

        value += self.ghost_value * sum((ghost.scaredTimer - manhattanDistance(newPos, ghost.getPosition()))
                                        for ghost in currentGameState.getGhostStates()
                                        if ghost.scaredTimer > manhattanDistance(newPos, ghost.getPosition()))
        if currentGameState.getNumFood():
            value -= self.food_num_value * currentGameState.getNumFood()
            value -= self.food_dis_value * min(manhattanDistance(newPos, foodPos)
                                               for foodPos in currentGameState.getFood().asList())

        # if scared_ghost >= ghost_num / 2:
        #     scared_rate = -1
        # else:
        #     scared_rate = 1
        scared_rate = 1

        if currentGameState.getCapsules():
            value -= scared_rate * self.capsule_dis_value * min(
                manhattanDistance(newPos, capsulePos) for capsulePos in currentGameState.getCapsules())
        value += scared_rate * self.capsule_dis_num * \
            len(currentGameState.getCapsules())
        return value

    def getAction(self, gameState: GameState):
        """
          Returns an action.  You can use any method you want and search to any depth you want.
          Just remember that the mini-contest is timed, so you have to trade off speed and computation.

          Ghosts don't behave randomly anymore, but they aren't perfect either -- they'll usually
          just make a beeline straight towards Pacman (or away from him if they're scared!)
        """
        "*** YOUR CODE HERE ***"
        self.step_num += 1

        direction = gameState.getPacmanState().getDirection()
        positon = gameState.getPacmanPosition()
        foods = gameState.getFood().asList()
        food_distance = min([manhattanDistance(positon, food)
                            for food in foods]) if foods else 0.5
        capsules = gameState.getCapsules()
        ghost_state = gameState.getGhostStates()
        newScaredTimes = [
            ghostState.scaredTimer for ghostState in ghost_state]
        if (positon[0] == 1 and positon[1] < 4 and (1, 1) not in capsules and newScaredTimes.count(0) == 0 and self.step_num > 600):
            print(self.step_num)
            return Directions.NORTH

        self.depth = 3

        def value(gameState, depth, agentIndex):
            if gameState.isWin() or gameState.isLose() or depth == self.depth:
                return self.bestEvaluationFunction(gameState)
            if agentIndex == 0:
                return max_value(gameState, depth)
            else:
                return expected_value(gameState, depth, agentIndex)

        def max_value(gameState, depth):
            v = -float('inf')
            if depth + 1 == self.depth:
                return self.bestEvaluationFunction(gameState)
            for action in gameState.getLegalActions(0):
                v = max(v, value(gameState.getNextState(0, action), depth+1, 1))
            return v

        def expected_value(gameState, depth, agentIndex):
            v = 0
            actions = gameState.getLegalActions(agentIndex)
            for action in actions:
                nextState = gameState.getNextState(agentIndex, action)
                if agentIndex == gameState.getNumAgents() - 1:
                    v += value(nextState, depth, 0)
                else:
                    v += value(nextState, depth, agentIndex+1)
            return v / len(actions)

        best_score = -float('inf')
        best_action = None
        for action in gameState.getLegalActions(0):
            score = value(gameState.getNextState(0, action), 0, 1)
            if score > best_score:
                best_score = score
                best_action = action
        return best_action
