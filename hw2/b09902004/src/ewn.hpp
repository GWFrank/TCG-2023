#ifndef EWN_HPP
#define EWN_HPP

#include <cstdint>

namespace ewn {

// Game environment
constexpr int ROW = 6;
constexpr int COL = 7;
constexpr int MAX_CUBES = 6;
constexpr int PERIOD = 21;  // the period of the given dice sequence
constexpr int RED = 0;
constexpr int BLUE = 1;

// Constraints (might be tunable)
constexpr int MAX_PLIES = 150;  // the average ply of a game is far smaller
constexpr int MAX_MOVES = 8;

class State {
    int board_[ROW * COL];
    int pos_[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int num_cubes_[2];
    int next_;  // next player

    int dice_seq_[PERIOD];
    int history_[MAX_PLIES];
    int n_plies_;

   public:
    void init_board();
    bool is_over();
    int move_gen_all(int *move_arr);
    void do_move(int move);
    void undo();

    int greedy();
    friend int search(State &, int alpha, int beta, int depth);
};

int search_and_get_move(State &, int depth);
int get_random_move(State &);

}  // namespace ewn

#endif
