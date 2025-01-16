from abc import abstractmethod
from .. import constants as c
from source.agents.env import GameState
from kanren import Relation, facts, run, conde, var
import random
import queue
import pdb
import numpy as np
import sys

sun_cost = {
    c.SUNFLOWER : 50,
    c.PEASHOOTER: 100,
    c.WALLNUT : 50,
    c.CHERRYBOMB: 150
}

class Action:
    def __init__(self, plant_name, cost, x, y):
        self.plant_cost = cost
        self.plant_name = plant_name
        self.x = x
        self.y = y

    def __str__(self):
        return f"{self.plant_name}, ({self.x}, {self.y})"


class Agent:
    def __init__(self, agentType=c.MOUSE_AGENT):
        self.agentType = agentType
        self.play_time = 0
        self.play_interval = 200

    @abstractmethod
    def getAction(self, state: GameState, current_time):
        """abstract method"""

    @abstractmethod
    def reflex(self, state: GameState):
        """abstract method"""


class RandomAgent(Agent):
    def getAction(self, state: GameState, current_time):
        gameState = state.getGameState()
        if gameState == "end":
            return
        sun_value = gameState["sun_value"]
        plant_availability = gameState["plant_availability"]  # [(plant_name, frozen_time, sun_cost), ..., ]
        grid_state = gameState["grid_state"] # 5*10 list, entry: [ (plant_name, hp), zombie_hp ]
        if current_time - self.play_time >= self.play_interval:
            self.play_time = current_time
            available_plant = []
            available_coordinate = []
            for plant_data in plant_availability:
                if plant_data[1] == 0 and sun_value > plant_data[2]:
                    available_plant.append(plant_data)
            for i in range(5):
                for j in range(9):
                    if grid_state[i][j][0][0] == c.BLANK:
                        available_coordinate.append((i, j))
            if len(available_coordinate) > 0 and len(available_plant) > 0 and random.random() < 0.5:
                plant = random.choice(available_plant)
                coordinate = random.choice(available_coordinate)
                return Action(plant[0], plant[2], coordinate[0], coordinate[1])
            else:
                return Action(c.IDLE, 0, 0, 0)
        else:
            return Action(c.IDLE, 0, 0, 0)



class LocalAgent(Agent):
    def getAction(self, state: GameState, current_time):
        gameState = state.getGameState()
        if gameState == "end":
            return
        sun_value = gameState["sun_value"]
        plant_availability = gameState["plant_availability"]  # [(plant_name, frozen_time, sun_cost), ..., ]
        grid_state = gameState["grid_state"] # 5*10 list, entry: [ (plant_name, hp), zombie_hp ]
        discount = 0.8
        if current_time - self.play_time < self.play_interval:
            return Action(c.IDLE, 0, 0, 0)
        self.play_time = current_time
        available_plant = []
        available_coordinate = []
        total_sunflowers = 0
        for plant_data in plant_availability:
            if plant_data[1] == 0 and sun_value > plant_data[2]:
                available_plant.append(plant_data)
        plants_count = [{plant[0]:0 for plant in plant_availability} for __ in range(5)]
        #First we compute each line the index 0 box to check whether the value is less than some fix threshold
        choose = 0
        least = 99999999999
        gama = 0.9
        plants = {each[0]:(each[1], each[2]) for each in plant_availability}
        values = [[0 for __ in range(9)] for _ in range(5)]
        for i in range(5):
            value = 0
            #For this process, we tranverse from right to left for each single line
            for j in range(8, -1, -1):
                grid = grid_state[i][j]
                plant_name = grid_state[i][j][0][0]
                #there is a discount factor gama, the equation is: V'(t) = V(t) + gama*V(t+1), we then use value iteration to compute value
                value *= gama
                
                if plant_name != c.BLANK:
                    # pdb.set_trace()
                    plants_count[i][plant_name] += 1
                if plant_name == c.SUNFLOWER:
                    total_sunflowers += 1
                if plant_name == c.BLANK:
                    value += grid[1]
                if plant_name == c.WALLNUT:
                    value /= grid[0][1]/10
                if plant_name == c.PEASHOOTER:
                    value -= 10
                if value < 0:
                    value = 0
                values[i][j] = value
        values = np.array(values)
        
        
        max_value = max(values[:, 0])
        min_value = min(values[:, 0])
            
        #If bigger, then turn into defence action
        thres = 5
        if max_value >= thres or total_sunflowers >= 10:
            store = queue.PriorityQueue()
            choose_line = 0
            least = 0
            for i in range(5):
                #First we choose a line that's most dangerous to handle
                if values[i][0] > least:
                    choose_line = i
                    least = values[i][0]
            #If there are lines that have zombie we can't kill, then plant peashoter first
            if least > 0:
                for j in range(9):
                    if grid_state[choose_line][j][0][0] == c.BLANK:
                        #After planting the peashoter or wallnut, we first try to compare the index 0 value
                        new_line_value = [0 for ___ in range(9)]
                        #Peashoter
                        for k in range(9):
                            grid = grid_state[choose_line][k]
                            plant_name = grid_state[choose_line][k][0][0]
                            value = 0
                            #there is a discount factor gama, the equation is: V'(t) = V(t) + gama*V(t+1), we then use value iteration to compute value
                            value *= gama
                            if plant_name != c.BLANK:
                                # pdb.set_trace()
                                plants_count[choose_line][plant_name] += 1
                            if plant_name == c.SUNFLOWER:
                                total_sunflowers += 1
                            if plant_name == c.BLANK:
                                value += grid[1]
                            if plant_name == c.WALLNUT:
                                value /= grid[0][1]/30
                            if plant_name == c.PEASHOOTER:
                                value -= 10
                            if k == j:
                                value -= 10
                            if value < 0:
                                value = 0
                            new_line_value[k] = value
                        if values[choose_line][0] - new_line_value[0] > 0:
                            store.put((values[choose_line][0] - new_line_value[0], c.PEASHOOTER, j, choose_line))
                        #If all values are equal to zero, then we use the average value among one single line
                        #Also if this has no change, then we try to plant the peashoter
                #If we dicide to plant peashoter
                if store.empty() == False:
                    action = store.get()
                    if action[1] == c.PEASHOOTER and sun_value >= 100 and plants[c.PEASHOOTER][0] == 0:
                        return Action(c.PEASHOOTER, 100, action[2], action[3])
            
            #Else we dicide to plant wallnut
            #Similar to before, we assume each blank box to have a wallnut, then compare the average value in a line to dicide where to put
            else:
                zombies = [0 for _ in range(5)]
                for i in range(5):
                    for j in range(9):
                        zombies[i] += grid_state[i][j][1]
                lease = 99999
                choose_line = 0
                for i in range(5):
                    if lease < zombies[i]:
                        lease = zombies[i]
                        choose_line = i
                for j in range(9):
                    if grid_state[choose_line][j][1] > 0 and sun_value >= 50 and plants[c.WALLNUT][0] == 0:
                        return Action(c.WALLNUT, 50, j, choose_line)
                
            return Action(c.IDLE, 0, 0, 0)
        #Else smaller we turn into preparation action
        else:
            #If we enter preparation action, we choose a line that has the smallest sunflowers or the least dangerous, to plant a sunflower
            store = queue.PriorityQueue()
            for i in range(5):
                for j in range(9):
                    if grid_state[i][j][0][0] == c.BLANK:
                        store.put((j+values[i][j], (i, j)))
                        break
            
            if sun_value >=50 and plants[c.SUNFLOWER][0] == 0:
                place = store.get()[1]
                return Action(c.SUNFLOWER, 50, place[1], place[0])
            else:
                return Action(c.IDLE, 0, 0, 0)
    
    def reflex(self, state: GameState):
        # TODO
        ...


class LogicAgent(Agent):
    def __init__(self, agent_type):
        super().__init__(agent_type)
        self.zombie_at = Relation()
        self.plant_at = Relation()
        self.action_for_alert_level = Relation()
        self.plant_for_sun_state = Relation()

        facts(self.action_for_alert_level, ("relax", c.SUNFLOWER), 
                                         ("alert", c.PEASHOOTER), 
                                         ("defense", c.WALLNUT), 
                                         ("critical", c.CHERRYBOMB))

        facts(self.plant_for_sun_state, ("Nothing", c.IDLE),
                                        ("poor", c.SUNFLOWER), 
                                        ("medium", c.SUNFLOWER), 
                                        ("medium", c.PEASHOOTER), 
                                        ("medium", c.WALLNUT),
                                        ("rich", c.SUNFLOWER), 
                                        ("rich",  c.PEASHOOTER), 
                                        ("rich", c.WALLNUT), 
                                        ("rich", c.CHERRYBOMB))

    def update_KB(self, gameState):
        self.zombie_at = Relation()
        self.plant_at = Relation()

        for row in range(len(gameState["grid_state"])):
            for col in range(len(gameState["grid_state"][row])):
                plant, zombie_hp = gameState["grid_state"][row][col]
                if zombie_hp > 0:
                    facts(self.zombie_at, (row, col))
                if plant[0] != c.BLANK:
                    facts(self.plant_at, (row, col))

    def alarm_system(self, grid_state, sun_value, plant_availability):
        weights = [8, 6, 3, 2, 1.5, 1.5, 1, 0.5, 0.4, 0.3]
        alert_scores = [0] * len(grid_state)

        for row in range(len(grid_state)):
            for col in range(len(grid_state[row])):
                _, zombie_hp = grid_state[row][col]
                if  0 < zombie_hp < 15:
                    alert_scores[row] += weights[col]
                elif zombie_hp >= 15:
                    alert_scores[row] += weights[col] * 1.2
                
        

        highest_alert_score = max(alert_scores)
        if highest_alert_score <= 0:
            return 'relax'
        elif 0.5 <= highest_alert_score <= 3:
            if sun_value < 100:
                return "defense"
            return 'alert'
        elif highest_alert_score == 4:
            if plant_availability[3][1] != 0:
                return "defense"
            return 'defense'
        elif highest_alert_score > 4:
            if plant_availability[1][1] != 0:
                return "defense"
            return 'critical'
        return 'relax'

    def get_sun_state(self, sun_value):
        if sun_value < 50:
            return "Nothing"
        elif sun_value == 50:
            return 'poor'
        elif 50 <= sun_value < 100:
            return 'medium'
        elif 100 <= sun_value:
            return 'rich'
        else:
            return 'none'

    def plant_available(self, plant_name, plant_availability):
        for plant, frozen_time, _ in plant_availability:
            if plant == plant_name and frozen_time == 0:
                return True
        return False

    
    def get_plant_position(self, grid_state, alert_level):
        print(alert_level)
        if alert_level == 'relax':
            alert_scores = [0] * len(grid_state)
            #print("列状态检查：")
            #for col in range(len(grid_state[0])):
            #    print(f"列 {col}: {grid_state[0][col]}")
            # 计算每一行的警报分数
            for row in range(len(grid_state)):
                sunflower_num = 0
                for col in range(len(grid_state[row])):
                    plant_group, zombie_hp = grid_state[row][col]
                    plant_name, plant_hp = plant_group
                    if plant_name == c.SUNFLOWER:
                        sunflower_num += 1 

                alert_scores[row] -= sunflower_num 
            print(alert_scores)
            chosen_row = alert_scores.index(max(alert_scores))
            for col in range(9):
                plant, _ = grid_state[chosen_row][col]
                if plant[0] == c.BLANK:
                    return chosen_row,col

        elif alert_level == 'alert':
            # 对于 'relax' 和 'alert'，选择最近的空列（从第一列开始）
            weights = [5, 4, 3, 2, 1.5, 1.5, 1, 0.5, 0.4, 0.3]
            alert_scores = [0] * len(grid_state)
            #print("列状态检查：")
            #for col in range(len(grid_state[0])):
            #    print(f"列 {col}: {grid_state[0][col]}")
            # 计算每一行的警报分数
            for row in range(len(grid_state)):
                peashooter_num = 0
                sunflower_num = 0
                for col in range(len(grid_state[row])):
                    plant_group, zombie_hp = grid_state[row][col]
                    plant_name, plant_hp = plant_group
                    if zombie_hp > 0:
                        alert_scores[row] += weights[col]
                    if plant_name == c.SUNFLOWER:
                        sunflower_num += plant_hp * 0.1
                    elif plant_name == c.PEASHOOTER:
                        peashooter_num += plant_hp * 0.1
                if peashooter_num == 0:
                    alert_scores[row] += 4
                alert_scores[row] -= sunflower_num * 0.05
                alert_scores[row] -= peashooter_num * 0.5

            # 选择警报程度最高的行
            print(alert_scores)
            chosen_row = alert_scores.index(max(alert_scores))
            for col in range(9):
                plant, _ = grid_state[chosen_row][col]
                if plant[0] == c.BLANK:
                    return chosen_row,col
        else:
            weights = [5, 4, 3, 2, 1, 0.9, 0.8, 0.5, 0.4, 0.3]
            alert_scores = [0] * len(grid_state)
            #print("列状态检查：")
            #for col in range(len(grid_state[0])):
            #    print(f"列 {col}: {grid_state[0][col]}")
            # 计算每一行的警报分数
            for row in range(len(grid_state)):
                for col in range(len(grid_state[row])):
                    _, zombie_hp = grid_state[row][col]
                    if zombie_hp > 0:
                        alert_scores[row] += weights[col]

            # 选择警报程度最高的行
            print(alert_scores)
            chosen_row = alert_scores.index(max(alert_scores))
            # 对于 'defense' 和 'critical'，选择有僵尸的列
            for col in range(9):
                _, zombie_hp = grid_state[chosen_row][col]
                if zombie_hp > 0:
                    return chosen_row, col

        return None  # 如果没有找到合适的位置，返回 None



    def getAction(self, state: GameState, current_time):
        gameState = state.getGameState()
        if gameState == "end":
            return
        sun_value = gameState["sun_value"]
        plant_availability = gameState["plant_availability"]  # [(plant_name, frozen_time, sun_cost), ..., ]
        grid_state = gameState["grid_state"] # 5*10 list, entry: [ (plant_name, hp), zombie_hp ]
        if current_time - self.play_time < self.play_interval:
            return Action(c.IDLE, 0, 0, 0)
        
        self.play_time = current_time

        self.update_KB(gameState)
        alert_level = self.alarm_system(grid_state, sun_value, plant_availability)
        sun_state = self.get_sun_state(sun_value)
        
        action = var()
        possible_actions = run(0, action, self.action_for_alert_level(alert_level, action))
        # print(possible_actions)
        x=var()
        for act in possible_actions:
            # print(act)
            # print(f"plant, {run(0, x, self.plant_for_sun_state(sun_state, x))}")
            # print(self.plant_available(act, plant_availability))
            # print(self.get_plant_position(grid_state, alert_level))
            # print('警报')
            # print(alert_level)
            if act in run(0, x, self.plant_for_sun_state(sun_state, x)) and \
               self.plant_available(act, plant_availability) and sun_value >= sun_cost[act]:
                position = self.get_plant_position(grid_state, alert_level)
                if position is None:
                    print(possible_actions)
                if position is not None:
                    row, col = position
                    if act == c.SUNFLOWER:
                        cost = 50
                    elif act == c.PEASHOOTER:
                        cost = 100
                    elif act == c.WALLNUT:
                        cost = 50
                    elif act == c.CHERRYBOMB:
                        cost = 175
                    
                    print(act, row, col)
                    return Action(act, cost, row,col)
        return Action(c.IDLE, 0, 0, 0)

class DQNAgent(Agent):
    def getAction(self, state: GameState, current_time):
        # TODO
        ...

    def reflex(self, state: GameState):
        # TODO
        ...

    # some functions

# We assume each line to be a Q-learning and compute value for each box
# Give each box a value and search the maximum step
# 


class LLMQAgent(Agent):
    def getAction(self, state: GameState, current_time):
        # TODO
        ...

    def reflex(self, state: GameState):
        # TODO
        ...