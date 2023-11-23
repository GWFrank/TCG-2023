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
    memset(m_board, 0xff, sizeof(m_board));
    memset(m_pos, 0xff, sizeof(m_pos));
    m_num_cubes[0] = MAX_CUBES;
    m_num_cubes[1] = MAX_CUBES;
    m_next = RED;
    m_n_plies = 0;

    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            int cube = getchar() - '0';
            m_board[init_pos[player][i]] = cube + offset;
            m_pos[cube + offset] = init_pos[player][i];
        }
        offset += MAX_CUBES;
    }
    for (int i = 0; i < PERIOD; i++) {
        m_dice_seq[i] = getchar() - '0';
    }
}

bool State::is_over() {
    if (m_num_cubes[0] == 0 || m_num_cubes[1] == 0) return true;
    if (is_blue_cube(m_board[0])) return true;
    if (is_red_cube(m_board[ROW * COL - 1])) return true;
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
    const int dice = m_dice_seq[m_n_plies % PERIOD];
    const int offset = m_next == BLUE ? MAX_CUBES : 0;
    int *const self_pos = m_pos + offset;

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
    int dst = m_pos[cube] + dir_val[m_next][direction];

    if (m_n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (m_board[dst] >= 0) {
        if (is_red_cube_fast(m_board[dst]))
            m_num_cubes[RED]--;
        else
            m_num_cubes[BLUE]--;
        m_pos[m_board[dst]] = -1;
        move |= m_board[dst] << 8;
    } else
        move |= 0xf00;
    m_board[m_pos[cube]] = -1;
    m_board[dst] = cube;
    m_pos[cube] = dst;
    m_history[m_n_plies++] = move;
    change_player(m_next);
}

void State::undo() {
    if (m_n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }
    change_player(m_next);

    int move = m_history[--m_n_plies];
    int eaten_cube = move >> 8;
    int cube = (move & 0xff) >> 4;
    int direction = move & 0xf;
    int src = m_pos[cube] - dir_val[m_next][direction];

    if (!is_empty_cube(eaten_cube)) {
        if (is_red_cube_fast(eaten_cube))
            m_num_cubes[RED]++;
        else
            m_num_cubes[BLUE]++;
        m_board[m_pos[cube]] = eaten_cube;
        m_pos[eaten_cube] = m_pos[cube];
    } else
        m_board[m_pos[cube]] = -1;
    m_board[src] = cube;
    m_pos[cube] = src;
}

inline int get_random_move(State &game) {
    int move_arr[MAX_MOVES];
    int num_moves = game.move_gen_all(move_arr);
    return move_arr[arc4random_uniform(num_moves)];
}

}  // namespace ewn
