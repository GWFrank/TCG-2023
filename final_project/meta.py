import math

board_size = 5

def l_inf_distance(pos1: int, pos2: int) -> int:
    x1, y1 = pos1 % board_size, pos1 // board_size
    x2, y2 = pos2 % board_size, pos2 // board_size
    return max(abs(x1 - x2), abs(y1 - y2))

# Generate a 2D array of distances
distances = [[l_inf_distance(i, j) for j in range(board_size**2)] for i in range(board_size**2)]

# Print the array in C format
print(f"int distances[{board_size**2}][{board_size**2}] = ")
print("{")
for i in range(board_size**2):
    print("{", end="")
    for j in range(board_size**2):
        print(distances[i][j], end="")
        if j != board_size**2 - 1:
            print(", ", end="")
    print("},")
print("};")
