// Modified from src/ewn.cpp in hw1_verifier
#include <algorithm>
#include <string>

#include <cstdio>
#include <cstdint>
#include <cstring>

#include "ewn.hpp"

namespace ewn {

// These are initialized after scanning the board
int ROW = -1;
int COL = -1;
int dir_value[8] = {-1};

Game::Game() {
    row = 0;
    col = 0;
    pos[0] = 999;
    pos[MAX_PIECES + 1] = 999;
    for (int i = 1; i <= MAX_PIECES; i++) {
        pos[i] = -1;
    }
    period = 0;
    goal_piece = 0;
    n_plies = 0;
}

Game::Game(const Game &rhs) {
    this->row = rhs.row;
    this->col = rhs.col;
    std::memcpy(this->board, rhs.board, MAX_ROW * MAX_COL * sizeof(int));
    std::memcpy(this->pos, rhs.pos, (MAX_PIECES + 2) * sizeof(int));
    std::memcpy(this->dice_seq, rhs.dice_seq, (MAX_PERIOD) * sizeof(int));
    this->period = rhs.period;
    this->goal_piece = rhs.goal_piece;
    std::memcpy(this->history, rhs.history, (MAX_PLIES) * sizeof(int));
    this->n_plies = rhs.n_plies;
}

Game &Game::operator=(const Game &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->row = rhs.row;
    this->col = rhs.col;
    std::memcpy(this->board, rhs.board, MAX_ROW * MAX_COL * sizeof(int));
    std::memcpy(this->pos, rhs.pos, (MAX_PIECES + 2) * sizeof(int));
    std::memcpy(this->dice_seq, rhs.dice_seq, (MAX_PERIOD) * sizeof(int));
    this->period = rhs.period;
    this->goal_piece = rhs.goal_piece;
    std::memcpy(this->history, rhs.history, (MAX_PLIES) * sizeof(int));
    this->n_plies = rhs.n_plies;
    return *this;
}

void set_dir_value() {
    dir_value[0] = -COL - 1;
    dir_value[1] = -COL;
    dir_value[2] = -COL + 1;
    dir_value[3] = -1;
    dir_value[4] = 1;
    dir_value[5] = COL - 1;
    dir_value[6] = COL;
    dir_value[7] = COL + 1;
}

void Game::scanBoard() {
    scanf(" %d %d", &this->row, &this->col);
    for (int i = 0; i < this->row * this->col; i++) {
        scanf(" %d", &this->board[i]);
        if (this->board[i] > 0) {
            this->pos[this->board[i]] = i;
        }
    }
    scanf(" %d", &this->period);
    for (int i = 0; i < this->period; i++) {
        scanf(" %d", &this->dice_seq[i]);
    }
    scanf(" %d", &this->goal_piece);

    // initialize global variables
    ROW = row;
    COL = col;
    set_dir_value();
}

void Game::printBoard() {
    for (int i = 0; i < this->row; i++) {
        for (int j = 0; j < this->col; j++) {
            fprintf(stderr, "%4d", this->board[i * this->col + j]);
        }
        fprintf(stderr, "\n");
    }
    // for (int i=1; i<=6; i++) {
    //     fprintf(stderr, "piece %d@%d ", i, pos[i]);
    // }
    fprintf(stderr, "\n");
}

void Game::printHistory() {
    printf("%d\n", this->n_plies);
    for (int i = 0; i < this->n_plies; i++) {
        int piece = (this->history[i] >> 4) & 0xf;
        int dir = (this->history[i]) & 0xf;
        printf("%d %d\n", piece, dir);
    }
}

bool Game::isGoal() {
    if (this->goal_piece == 0) {
        return this->board[this->row * this->col - 1] > 0;
    } else {
        return this->board[this->row * this->col - 1] == this->goal_piece;
    }
    return false;
}

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the piece number
11~8: store the eaten piece (used only in history)
*/

int move_gen2(int *moves, int piece, int location) {
    int count = 0;
    int row = location / COL;
    int col = location % COL;

    bool left_ok = col != 0;
    bool right_ok = col != COL - 1;
    bool up_ok = row != 0;
    bool down_ok = row != ROW - 1;

    if (up_ok) {
        moves[count++] = piece << 4 | 1;
    }
    if (left_ok) {
        moves[count++] = piece << 4 | 3;
    }
    if (right_ok) {
        moves[count++] = piece << 4 | 4;
    }
    if (down_ok) {
        moves[count++] = piece << 4 | 6;
    }

    if (up_ok && left_ok) {
        moves[count++] = piece << 4 | 0;
    }
    if (up_ok && right_ok) {
        moves[count++] = piece << 4 | 2;
    }
    if (down_ok && left_ok) {
        moves[count++] = piece << 4 | 5;
    }
    if (down_ok && right_ok) {
        moves[count++] = piece << 4 | 7;
    }

    return count;
}

int Game::generateAllMoves(int *moves) {
    int count = 0;
    int dice = this->dice_seq[this->n_plies % this->period];
    if (this->pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (this->pos[small] == -1) {
            small--;
        }
        while (this->pos[large] == -1) {
            large++;
        }

        if (small >= 1) {
            count += move_gen2(moves, small, this->pos[small]);
        }
        if (large <= MAX_PIECES) {
            count += move_gen2(moves + count, large, this->pos[large]);
        }
    } else {
        count = move_gen2(moves, dice, this->pos[dice]);
    }
    return count;
}

void Game::doMove(int move) {
    int piece = move >> 4;
    int direction = move & 15;
    int dst = this->pos[piece] + dir_value[direction];

    if (this->n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (this->board[dst] > 0) {  // eats a piece
        this->pos[this->board[dst]] = -1;
        move |= this->board[dst] << 8;
    }
    this->board[this->pos[piece]] = 0;
    this->board[dst] = piece;
    this->pos[piece] = dst;
    this->history[this->n_plies++] = move;
}

void Game::undo() {
    if (this->n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }

    int move = this->history[--this->n_plies];
    int eaten_piece = move >> 8;
    int piece = (move & 255) >> 4;
    int direction = move & 15;
    int dst = this->pos[piece] - dir_value[direction];

    if (eaten_piece > 0) {
        this->board[this->pos[piece]] = eaten_piece;
        this->pos[eaten_piece] = this->pos[piece];
    } else {
        this->board[this->pos[piece]] = 0;
    }
    this->board[dst] = piece;
    this->pos[piece] = dst;
}

// Position should be in [-1, 81], so 7 bits is enough to represent 1 piece
// we can fit all 6 pieces inside one 64-bit integer
uint64_t Game::hash() {
    uint64_t h = 0;
    for (int i = 1; i <= 6; i++) {
        uint64_t coord = 1 + this->pos[i];
        h |= (coord) << (7 * (i - 1));
    }
    return h;
}

bool Game::isDoable() {
    if (this->goal_piece == 0) {
        return true;
    }
    return this->pos[goal_piece] != -1;
}

bool Game::isImproving(int move) {
    // An optimal move should make the piece closer to the goal or other pieces
    int piece = (move >> 4) & 0xf;
    int direction = move & 0xf;
    int dst = this->pos[piece] + dir_value[direction];
    int x = this->pos[piece] % this->col, y = this->pos[piece] / this->col;
    int dst_x = dst % this->col, dst_y = dst / this->col;

    // Check if getting closer to any piece
    for (int i = 1; i <= 6; i++) {
        if (i == piece || this->pos[piece] == -1) {
            continue;
        }
        int i_x = this->pos[i] % this->col, i_y = this->pos[i] / this->col;
        int old_dist = std::max(std::abs(i_x - x), std::abs(i_y - y));
        int new_dist = std::max(std::abs(i_x - dst_x), std::abs(i_y - dst_y));
        if (new_dist < old_dist) {
            return true;
        }
    }
    // Check if getting closer to goal square
    int old_dist =
        std::max(std::abs((this->col - 1) - x), std::abs((this->row - 1) - y));
    int new_dist = std::max(std::abs((this->col - 1) - dst_x),
                            std::abs((this->row - 1) - dst_y));
    if (new_dist < old_dist) {
        return true;
    }
    return false;
}

bool Game::hasGoalPiece() { return this->goal_piece != 0; }

int Game::kingDistance(int piece) {
    if (this->pos[piece] == -1) {  // piece eaten, can't make it
        return (MAX_ROW + 1) * (MAX_COL + 1) * 2;
    }
    int x = this->pos[piece] % this->col;
    int y = this->pos[piece] / this->col;
    return std::max(std::abs((this->col - 1) - x),
                    std::abs((this->row - 1) - y));
}

int Game::currentCost() { return this->n_plies; }

int Game::heuristic() {
    if (this->goal_piece == 0) {
        int min_distance = __INT32_MAX__;
        for (int i = 1; i <= 6; i++) {
            min_distance = std::min(min_distance, this->kingDistance(i));
        }
        return min_distance;

    } else {
        return this->kingDistance(this->goal_piece);
    }
}

}  // namespace ewn
