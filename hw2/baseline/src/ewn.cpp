#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ewn.h"

#define is_red_cube(x) ((x) >= 0 && (x) < (MAX_CUBES))
#define is_blue_cube(x) ((x) >= (MAX_CUBES) && (x) < (MAX_CUBES) << 1)
#define is_red_cube_fast(x) ((x) < (MAX_CUBES))
#define is_blue_cube_fast(x) ((x) >= (MAX_CUBES))
#define is_empty_cube(x) ((x) == 0xf)
#define change_player(x) ((x) ^= 1)

static const int dir_val[2][4] = {{1, COL, COL+1, -COL+1}, {-1, -COL, -COL-1, COL-1}};
static const int init_pos[2][MAX_CUBES] = {
    {0, 1, 2, COL, COL+1, COL*2},
    {(ROW-2)*COL-1, (ROW-1)*COL-2, (ROW-1)*COL-1, ROW*COL-3, ROW*COL-2, ROW*COL-1}
};

void EWN::init_board() {
    memset(board, 0xff, sizeof(board));
    memset(pos, 0xff, sizeof(pos));
    num_cubes[0] = MAX_CUBES;
    num_cubes[1] = MAX_CUBES;
    next = RED;
    n_plies = 0;

    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            int cube = getchar() - '0';
            board[init_pos[player][i]] = cube + offset;
            pos[cube + offset] = init_pos[player][i];
        }
        offset += MAX_CUBES;
    }
    for (int i = 0; i < PERIOD; i++) {
        dice_seq[i] = getchar() - '0';
    }
}

bool EWN::is_over() {
    if (num_cubes[0] == 0 || num_cubes[1] == 0) return true;
    if (is_blue_cube(board[0])) return true;
    if (is_red_cube(board[ROW * COL - 1])) return true;
    return false;
}

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the cube number
11~8: store the eaten cube (used only in history)
*/

int move_gen(int *move_arr, int cube, int location) {
    int count = 0;
    const int row = location / COL;
    const int col = location % COL;
    bool h_ok;  // horizontal
    bool v_ok;  // vertical
    bool rev_v_ok;  // reverse, vertical

    if (is_red_cube_fast(cube)) {
        h_ok = col != COL - 1;
        v_ok = row != ROW - 1;
        rev_v_ok = row != 0;
    }
    else {
        h_ok = col != 0;
        v_ok = row != 0;
        rev_v_ok = row != ROW - 1;
    }
    if (h_ok) move_arr[count++] = cube << 4;
    if (v_ok) move_arr[count++] = cube << 4 | 1;
    if (h_ok && v_ok) move_arr[count++] = cube << 4 | 2;
    if (h_ok && rev_v_ok) move_arr[count++] = cube << 4 | 3;

    return count;
}

/*
int move_gen2(int *moves, int piece, int location) {
    int count = 0;
    int row = location / COL;
    int col = location % COL;

    bool left_ok = col != 0;
    bool right_ok = col != COL - 1;
    bool up_ok = row != 0;
    bool down_ok = row != ROW - 1;

    if (up_ok) moves[count++] = piece << 4 | 1;
    if (left_ok) moves[count++] = piece << 4 | 3;
    if (right_ok) moves[count++] = piece << 4 | 4;
    if (down_ok) moves[count++] = piece << 4 | 6;

    if (up_ok && left_ok) moves[count++] = piece << 4 | 0;
    if (up_ok && right_ok) moves[count++] = piece << 4 | 2;
    if (down_ok && left_ok) moves[count++] = piece << 4 | 5;
    if (down_ok && right_ok) moves[count++] = piece << 4 | 7;

    return count;
}
*/

int EWN::move_gen_all(int *move_arr) {
    int count = 0;
    const int dice = dice_seq[n_plies % PERIOD];
    const int offset = next == BLUE ? MAX_CUBES : 0;
    int * const self_pos = pos + offset;

    if (self_pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (small >= 0 && self_pos[small] == -1) small--;
        while (large < MAX_CUBES && self_pos[large] == -1) large++;

        if (small >= 0)
            count += move_gen(move_arr, small + offset, self_pos[small]);
        if (large < MAX_CUBES)
            count += move_gen(move_arr + count, large + offset, self_pos[large]);
    }
    else {
        count = move_gen(move_arr, dice + offset, self_pos[dice]);
    }

    return count;
}

void EWN::do_move(int move) {
    int cube = move >> 4;
    int direction = move & 0xf;
    int dst = pos[cube] + dir_val[next][direction];

    if (n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (board[dst] >= 0) {
        if (is_red_cube_fast(board[dst])) num_cubes[RED]--;
        else num_cubes[BLUE]--;
        pos[board[dst]] = -1;
        move |= board[dst] << 8;
    }
    else move |= 0xf00;
    board[pos[cube]] = -1;
    board[dst] = cube;
    pos[cube] = dst;
    history[n_plies++] = move;
    change_player(next);
}

void EWN::undo() {
    if (n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }
    change_player(next);

    int move = history[--n_plies];
    int eaten_cube = move >> 8;
    int cube = (move & 0xff) >> 4;
    int direction = move & 0xf;
    int src = pos[cube] - dir_val[next][direction];

    if (!is_empty_cube(eaten_cube)) {
        if (is_red_cube_fast(eaten_cube)) num_cubes[RED]++;
        else num_cubes[BLUE]++;
        board[pos[cube]] = eaten_cube;
        pos[eaten_cube] = pos[cube];
    }
    else board[pos[cube]] = -1;
    board[src] = cube;
    pos[cube] = src;
}
