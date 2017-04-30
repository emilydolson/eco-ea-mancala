import random
import numpy as np

for _ in range(500):
    board = [0]*14
    endcell = random.randint(1, 6)
    opposite = abs(7-(endcell%7))+7
    board[endcell] = 0
    correct = random.randint(1, 6)
    while correct == endcell:
        correct = random.randint(1, 6)
    
    if correct < endcell:
        board[correct] = endcell-correct
    else:
        board[correct] = 7 + (6-correct) + endcell

    remaining = 48 - board[correct]

    board[opposite] = int(np.random.power(.6)*min(16, remaining))
    while board[opposite] == 0:
        board[opposite] = int(np.random.power(.6)*min(16, remaining))

    remaining -= board[opposite]

    cells = list(range(14))
    random.shuffle(cells)
    for i in cells:
        if i == correct or i == endcell or i == opposite:
            continue

        stones = int(np.random.power(.6)*min(16, remaining))
        if stones > remaining:
            stones = remaining

        board[i] = stones
        remaining -= stones

    for i in range(1, 7):
        if board[i] == 0 and i != endcell:
            #we need to make sure this isn't a capture move
            opposite = abs(7-(i%7))+7            
            if not board[opposite]:
                continue
            for j in range(i):
                if board[j] == i-j:
                    board[j] -= 1
                    board[random.randint(0,1)] += 1

            for j in range(i+1, 6):
                if board[j] == 7 + (6-j) + i:
                    board[j] -= 1
                    board[random.randint(0,1)] += 1

        elif board[i] == 13:
            opposite = abs(7-(i%7))+7            
            if not board[opposite]:
                continue
            board[i] -= 1
            board[random.randint(0,1)] += 1
            
        
    if remaining > 0:
        cell0 = random.randint(0, remaining)
        board[0] += cell0
        board[7] += remaining - cell0

    print(",".join([str(i) for i in board]) + ","+str(correct))
