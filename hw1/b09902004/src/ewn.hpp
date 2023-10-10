// Modified from src/ewn.h in hw1_verifier
#ifndef EWN_HPP
#define EWN_HPP

#include <string>

namespace ewn {

const int MAX_ROW = 9;
const int MAX_COL = 9;
const int MAX_PIECES = 6;
const int MAX_PERIOD = 18;
const int MAX_PLIES = 100;
const int MAX_MOVES = 16;

// These are initialized after scanning the board
extern int ROW;
extern int COL;
extern int dir_value[8];

class Game {
    int row, col;
    int board[MAX_ROW * MAX_COL];
    int pos[MAX_PIECES + 2];  // pos[0] and pos[MAX_PIECES + 1] are not used
    int dice_seq[MAX_PERIOD];
    int period;
    int goal_piece;

    int history[MAX_PLIES];
    int n_plies;

   public:
    Game();
    Game(const Game& rhs);
    Game& operator=(const Game& rhs);

    void scan_board();
    void print_board();
    void print_history();
    bool is_goal();

    int move_gen_all(int *moves);
    void do_move(int move);
    void undo();

    u_int64_t hash();
    bool isDoable();
    int kingDistance(int piece);
    int currentCost();


    int heuristic();
    int heuristic2();
    void sort_move(int *moves, int n_move);
};

}  // namespace ewn

#endif
