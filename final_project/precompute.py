import math

BOARD_SIZE = 5
PIECE_NUM = 6
OFF_BOARD_POS = -1

def is_red_piece(piece: int) -> bool:
    return piece > PIECE_NUM and piece <= 2*PIECE_NUM

def is_blue_piece(piece: int) -> bool:
    return piece > 0 and piece <= PIECE_NUM

# Print the header
print("// This file is generated by precompute.py")
print("// Do not edit this file directly")
print("#ifndef PRECOMPUTE_H")
print("#define PRECOMPUTE_H")
print("#include \"agent.h\"")
print("namespace precompute {")

def l_inf_distance(pos1: int, pos2: int) -> int:
    x1, y1 = pos1 % BOARD_SIZE, pos1 // BOARD_SIZE
    x2, y2 = pos2 % BOARD_SIZE, pos2 // BOARD_SIZE
    return max(abs(x1 - x2), abs(y1 - y2))

def move_gen_single(piece: int, location: int) -> list:
    x, y = location % BOARD_SIZE, location // BOARD_SIZE
    moves = []
    h_ok: bool = False
    v_ok: bool = False
    if (is_red_piece(piece)):
        h_ok = x != BOARD_SIZE-1
        v_ok = y != BOARD_SIZE-1
    else:
        h_ok = x != 0
        v_ok = y != 0
    
    if h_ok:
        moves.append((piece << 4) | 0)
    if v_ok:
        moves.append((piece << 4) | 1)
    if h_ok and v_ok:
        moves.append((piece << 4) | 2)
    return moves

def moveable_pieces(
    dice: int,
    piece1_on_board: bool,
    piece2_on_board: bool,
    piece3_on_board: bool,
    piece4_on_board: bool,
    piece5_on_board: bool,
    piece6_on_board: bool,
) -> list[int]:
    # If the corresponding piece is not on the board, choose the first neighbors still on the board
    # If the corresponding piece is on the board, choose the corresponding piece

    on_board = [piece1_on_board, piece2_on_board, piece3_on_board, piece4_on_board, piece5_on_board, piece6_on_board]

    if on_board[dice - 1]:
        return [dice]
    else:
        small = dice - 1
        big = dice + 1
        while small > 0 and (not on_board[small - 1]):
            small -= 1
        while big <= PIECE_NUM and (not on_board[big - 1]):
            big += 1
        movables = []
        if small > 0:
            movables.append(small)
        if big <= PIECE_NUM:
            movables.append(big)
        return movables

def determinacy(
    piece1_on_board: bool,
    piece2_on_board: bool,
    piece3_on_board: bool,
    piece4_on_board: bool,
    piece5_on_board: bool,
    piece6_on_board: bool,
) -> list[int]:
    # Among all dice values, how often does each piece get to move?
    freq = [0, 0, 0, 0, 0, 0]
    for dice in range(1, PIECE_NUM + 1):
        movables = moveable_pieces(dice, piece1_on_board, piece2_on_board, piece3_on_board, piece4_on_board, piece5_on_board, piece6_on_board)
        for movable in movables:
            freq[movable - 1] += 1
    return freq
    
# print(move_gen_single(10, 1))
# exit(1)


# Print precomputed distances
print(f"constexpr int l_inf_distance[{BOARD_SIZE**2}][{BOARD_SIZE**2}] = ")
print("{")
for i in range(BOARD_SIZE**2):
    print("{", end="")
    for j in range(BOARD_SIZE**2):
        print(l_inf_distance(i, j), end="")
        if j != BOARD_SIZE**2 - 1:
            print(", ", end="")
    print("},")
print("};")


# Print precomputed move_gen_single
print(f"constexpr ewn::move_t move_gen_single[{2*PIECE_NUM+1}][{BOARD_SIZE*BOARD_SIZE}][3] = ")
print("{")
print("{},") # Skip the first row
for i in range(1, 2*PIECE_NUM+1):
    print("{", end="")
    for j in range(BOARD_SIZE*BOARD_SIZE):
        moves = move_gen_single(i, j)
        print("{", end="")
        for idx, move in enumerate(moves):
            print(move, end="")
            if idx != len(moves) - 1:
                print(", ", end="")
        print("}", end="")
        if j != BOARD_SIZE*BOARD_SIZE - 1:
            print(", ", end="")
    print("},")
print("};")

# Print precomputed length of move_gen_single
print(f"constexpr int move_gen_single_count[{2*PIECE_NUM+1}][{BOARD_SIZE*BOARD_SIZE}] = ")
print("{")
print("{},") # Skip the first row
for i in range(1, 2*PIECE_NUM+1):
    print("{", end="")
    for j in range(BOARD_SIZE*BOARD_SIZE):
        print(len(move_gen_single(i, j)), end="")
        if j != BOARD_SIZE*BOARD_SIZE - 1:
            print(", ", end="")
    print("},")
print("};")

# Print precomputed movables
print(f"constexpr int movable_pieces[{PIECE_NUM+1}][{2**PIECE_NUM}][2] = ")
print("{")
print("{},") # Skip the first row
for i in range(1, PIECE_NUM+1):
    print("{", end="")
    for j in range(2**PIECE_NUM):
        movables = moveable_pieces(i, j & 1, j & 2, j & 4, j & 8, j & 16, j & 32)
        print("{", end="")
        for idx, move in enumerate(movables):
            print(move, end="")
            if idx != len(movables) - 1:
                print(", ", end="")
        print("}", end="")
        if j != 2**PIECE_NUM - 1:
            print(", ", end="")
    print("},")
print("};")

# Print precomputed length of movables
print(f"constexpr int movable_pieces_count[{PIECE_NUM+1}][{2**PIECE_NUM}] = ")
print("{")
print("{},") # Skip the first row
for i in range(1, PIECE_NUM+1):
    print("{", end="")
    for j in range(2**PIECE_NUM):
        print(len(moveable_pieces(i, j & 1, j & 2, j & 4, j & 8, j & 16, j & 32)), end="")
        if j != 2**PIECE_NUM - 1:
            print(", ", end="")
    print("},")
print("};")

# Print precomputed determinacy

print(f"constexpr int determinacy[{2**PIECE_NUM}][{PIECE_NUM+1}] = ")
print("{")
for i in range(2**PIECE_NUM):
    freq = determinacy(i & 1, i & 2, i & 4, i & 8, i & 16, i & 32)
    print("{", end="")
    for idx, f in enumerate(freq):
        print(f, end="")
        if idx != len(freq) - 1:
            print(", ", end="")
    print("},")
print("};")

# Print the footer
print("} // namespace precompute")
print("#endif // PRECOMPUTE_H")