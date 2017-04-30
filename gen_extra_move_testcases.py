import random
import numpy as np

for _ in range(500):
    board = [0]*14
    correct = random.randint(1, 6)
    board[correct] = 7 - correct
    remaining = 48 - board[correct]
    cells = list(range(14))
    random.shuffle(cells)
    for i in cells:
        if i == correct:
            continue

        stones = int(np.random.power(.6)*min(16, remaining))
        if stones > remaining:
            stones = remaining

        #Only one correct move
        while i < 7 and ((7-i) == stones):
            stones = int(np.random.power(.6)*min(16, remaining))
            if stones > remaining:
                stones = remaining

        board[i] = stones
        remaining -= stones
        
    if remaining > 0:
        cell0 = random.randint(0, remaining)
        board[0] += cell0
        board[7] += remaining - cell0

    print(",".join([str(i) for i in board]) + ","+str(correct))
