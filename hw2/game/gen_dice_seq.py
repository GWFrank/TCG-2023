import sys
# from random import randint
from secrets import randbelow

if __name__ == "__main__":
    NUM_DICE_SEQ = 60  # the range of this variable is [20, 100]
    DICE_SEQ_LEN = 21

    with open("dice_seq.txt", "w") as fo:
        for i in range(NUM_DICE_SEQ):
            dice_seq = ""
            for j in range(DICE_SEQ_LEN):
                dice = randbelow(6)
                dice_seq += f" {dice}"
            print(dice_seq, file=fo)
