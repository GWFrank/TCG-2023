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
   private:
    int m_board[ROW * COL];
    int m_pos[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int m_num_cubes[2];
    int m_next;  // next player

    int m_dice_seq[PERIOD];
    int m_history[MAX_PLIES];
    int m_n_plies;

   public:
    void init_board();
    bool is_over();
    int move_gen_all(int *move_arr);
    void do_move(int move);
    void undo();
};

inline int get_random_move(const State &);

}  // namespace ewn

#endif
