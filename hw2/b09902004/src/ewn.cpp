#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ewn.hpp"

namespace ewn {

inline bool is_red_cube(int x) { return (x >= 0) && (x < MAX_CUBES); }

inline bool is_blue_cube(int x) {
    return (x >= MAX_CUBES) && (x < (MAX_CUBES << 1));
}

inline bool is_red_cube_fast(int x) { return x < MAX_CUBES; }

inline bool is_blue_cube_fast(int x) { return x >= MAX_CUBES; }

inline bool is_empty_cube(int x) { return x == 0xf; }

inline void change_player(int &x) { x ^= 1; }

// clang-format off
// -COL-1 | -COL | -COL+1
//     -1 |   0  |      1
//  COL-1 |  COL |  COL+1
static const int dir_val[2][4] = {
    { 1,  COL,  COL + 1, -COL + 1},
    {-1, -COL, -COL - 1,  COL - 1}
};

static const int init_pos[2][MAX_CUBES] = {
    {              0,               1,               2,         COL,     COL + 1,     COL * 2},
    {(ROW-2)*COL - 1, (ROW-1)*COL - 2, (ROW-1)*COL - 1, ROW*COL - 3, ROW*COL - 2, ROW*COL - 1}
};
// clang-format on

void State::init_board() {
    memset(board_, 0xff, sizeof(board_));
    memset(pos_, 0xff, sizeof(pos_));
    num_cubes_[0] = MAX_CUBES;
    num_cubes_[1] = MAX_CUBES;
    next_ = RED;
    n_plies_ = 0;

    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            int cube = getchar() - '0';
            board_[init_pos[player][i]] = cube + offset;
            pos_[cube + offset] = init_pos[player][i];
        }
        offset += MAX_CUBES;
    }
    for (int i = 0; i < PERIOD; i++) {
        dice_seq_[i] = getchar() - '0';
    }
}

bool State::is_over() {
    if (num_cubes_[0] == 0 || num_cubes_[1] == 0) return true;
    if (is_blue_cube(board_[0])) return true;
    if (is_red_cube(board_[ROW * COL - 1])) return true;
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
    bool h_ok;      // horizontal
    bool v_ok;      // vertical
    bool rev_v_ok;  // reverse, vertical

    if (is_red_cube_fast(cube)) {
        h_ok = col != COL - 1;
        v_ok = row != ROW - 1;
        rev_v_ok = row != 0;
    } else {
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

int State::move_gen_all(int *move_arr) {
    int count = 0;
    const int dice = dice_seq_[n_plies_ % PERIOD];
    const int offset = next_ == BLUE ? MAX_CUBES : 0;
    int *const self_pos = pos_ + offset;

    if (self_pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (small >= 0 && self_pos[small] == -1) small--;
        while (large < MAX_CUBES && self_pos[large] == -1) large++;

        if (small >= 0)
            count += move_gen(move_arr, small + offset, self_pos[small]);
        if (large < MAX_CUBES)
            count +=
                move_gen(move_arr + count, large + offset, self_pos[large]);
    } else {
        count = move_gen(move_arr, dice + offset, self_pos[dice]);
    }

    return count;
}

void State::do_move(int move) {
    int cube = move >> 4;
    int direction = move & 0xf;
    int dst = pos_[cube] + dir_val[next_][direction];

    if (n_plies_ == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (board_[dst] >= 0) {
        if (is_red_cube_fast(board_[dst]))
            num_cubes_[RED]--;
        else
            num_cubes_[BLUE]--;
        pos_[board_[dst]] = -1;
        move |= board_[dst] << 8;
    } else
        move |= 0xf00;
    board_[pos_[cube]] = -1;
    board_[dst] = cube;
    pos_[cube] = dst;
    history_[n_plies_++] = move;
    change_player(next_);
}

void State::undo() {
    if (n_plies_ == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }
    change_player(next_);

    int move = history_[--n_plies_];
    int eaten_cube = move >> 8;
    int cube = (move & 0xff) >> 4;
    int direction = move & 0xf;
    int src = pos_[cube] - dir_val[next_][direction];

    if (!is_empty_cube(eaten_cube)) {
        if (is_red_cube_fast(eaten_cube))
            num_cubes_[RED]++;
        else
            num_cubes_[BLUE]++;
        board_[pos_[cube]] = eaten_cube;
        pos_[eaten_cube] = pos_[cube];
    } else
        board_[pos_[cube]] = -1;
    board_[src] = cube;
    pos_[cube] = src;
}

}  // namespace ewn
