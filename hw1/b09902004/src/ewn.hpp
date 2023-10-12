// Modified from src/ewn.h in hw1_verifier
#ifndef EWN_HPP
#define EWN_HPP

#include <algorithm>
#include <vector>

#include <cstdint>

namespace ewn {

const int MAX_ROW = 9;
const int MAX_COL = 9;
const int MAX_PIECES = 6;
const int MAX_PERIOD = 18;
const int MAX_MOVES = 16;

// These are initialized after scanning the board
extern int ROW;
extern int COL;
extern int dir_value[8];

class Game {
    int row, col;
    int pos[MAX_PIECES + 2];  // pos[0] and pos[MAX_PIECES + 1] are not used
    int dice_seq[MAX_PERIOD];
    int period;
    int goal_piece;
    std::vector<int> history;
    int h;

    void calculateHeuristic();

   public:
    Game();
    Game(const Game& rhs);
    Game& operator=(const Game& rhs);
    bool operator<(const Game& rhs) const;

    void scanBoard();
    void printBoard();
    void printHistory();
    bool isGoal();

    int generateAllMoves(int* moves);
    void doMove(int move);
    void undo();

    uint64_t hash();
    bool isDoable();
    bool isImproving(int move);
    int kingDistance(int pos_a, int pos_b);
};

// inline member function should be put in header files

inline bool Game::isDoable() {
    if (this->goal_piece == 0) {
        return true;
    }
    return this->pos[this->goal_piece] != -1;
}

}  // namespace ewn

#endif
