// Modified from src/ewn.cpp in hw1_verifier
#include <algorithm>
#include <vector>

#include <cstdio>
#include <cstdint>
#include <cstring>

#include "ewn.hpp"

namespace ewn {

// These are initialized after scanning the board
int ROW = -1;
int COL = -1;
int dir_value[8] = {-1};
int dice_seq[MAX_PERIOD] = {0};

Game::Game() : history() {
    row = 0;
    col = 0;
    pos[0] = 999;
    pos[MAX_PIECES + 1] = 999;
    for (int i = 1; i <= MAX_PIECES; i++) {
        pos[i] = -1;
    }
    period = 0;
    goal_piece = 0;
    h = -1;
}

Game::Game(const Game &rhs) : history(rhs.history) {
    this->row = rhs.row;
    this->col = rhs.col;
    std::memcpy(this->pos, rhs.pos, (MAX_PIECES + 2) * sizeof(int));
    this->period = rhs.period;
    this->goal_piece = rhs.goal_piece;
    this->h = rhs.h;
}

Game &Game::operator=(const Game &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->row = rhs.row;
    this->col = rhs.col;
    std::memcpy(this->pos, rhs.pos, (MAX_PIECES + 2) * sizeof(int));
    this->period = rhs.period;
    this->goal_piece = rhs.goal_piece;
    this->history = rhs.history;
    this->h = rhs.h;
    return *this;
}

bool Game::operator<(const Game &rhs) const {
    return (this->history.size() + this->h) > (rhs.history.size() + rhs.h);
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
    int piece = -1;
    for (int i = 0; i < this->row * this->col; i++) {
        scanf(" %d", &piece);
        if (piece > 0) {
            this->pos[piece] = i;
        }
    }
    scanf(" %d", &this->period);
    for (int i = 0; i < this->period; i++) {
        scanf(" %d", &dice_seq[i]);
    }
    scanf(" %d", &this->goal_piece);

    // initialize global variables
    ROW = row;
    COL = col;
    set_dir_value();
    this->calculateHeuristic();
}

void Game::printBoard() {
    std::vector<int> bd(this->row * this->col, 0);
    for (int i = 1; i <= 6; i++) {
        if (this->pos[i] != -1) {
            bd[this->pos[i]] = i;
        }
    }
    for (int i = 0; i < this->row; i++) {
        for (int j = 0; j < this->col; j++) {
            fprintf(stderr, "%4d", bd[i * this->col + j]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void Game::printHistory() {
    printf("%ld\n", this->history.size());
    for (auto &move : this->history) {
        int piece = (move >> 4) & 0xf;
        int dir = (move) & 0xf;
        printf("%d %d\n", piece, dir);
    }
}

bool Game::isGoal() {
    if (this->goal_piece == 0) {
        for (int i = 1; i <= 6; i++) {
            if (this->pos[i] == this->row * this->col - 1) {
                return true;
            }
        }
        return false;
    } else {
        return this->pos[goal_piece] == this->row * this->col - 1;
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
    int dice = dice_seq[this->history.size() % this->period];
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

    // check if eating a piece
    for (int i = 1; i <= 6; i++) {
        if (i == piece) {
            continue;
        }
        if (this->pos[i] == dst) {
            this->pos[i] = -1;
            move |= i << 8;
            break;
        }
    }

    this->pos[piece] = dst;
    this->history.push_back(move);

    this->calculateHeuristic();
}

void Game::undo() {
    if (this->history.empty()) {
        fprintf(stderr, "no history\n");
        exit(1);
    }

    int move = this->history.back();
    this->history.pop_back();

    int eaten_piece = move >> 8;
    int piece = (move & 255) >> 4;
    int direction = move & 15;
    int dst = this->pos[piece] - dir_value[direction];

    if (eaten_piece > 0) {
        this->pos[eaten_piece] = this->pos[piece];
    }
    this->pos[piece] = dst;
}

// Position should be in [-1, 81], so 7 bits is enough to represent 1 piece
// we can fit all 6 pieces inside one 64-bit integer
hash_t Game::hash() {
    hash_t h(0);
    for (int i = 1; i <= 6; i++) {
        h |= static_cast<hash_t>(this->pos[i] + 1) << (7 * (i - 1));
    }
    return h;
}

bool Game::isImproving(int move) {
    // An optimal move should make the piece closer to the goal or other pieces
    int piece = (move >> 4) & 0xf;
    int direction = move & 0xf;
    int dst = this->pos[piece] + dir_value[direction];

    // Check if getting closer to any piece
    for (int i = 1; i <= 6; i++) {
        if (i == piece || this->pos[piece] == -1) {
            continue;
        }
        int old_dist = this->kingDistance(this->pos[piece], this->pos[i]);
        int new_dist = this->kingDistance(dst, this->pos[i]);
        if (new_dist < old_dist || (new_dist == old_dist && old_dist == 1)) {
            return true;
        }
    }
    // Check if getting closer to goal square
    int old_dist =
        this->kingDistance(this->pos[piece], this->row * this->col - 1);
    int new_dist = this->kingDistance(dst, this->row * this->col - 1);
    if (new_dist < old_dist) {
        return true;
    }
    return false;
}

void Game::calculateHeuristic() {
    if (this->goal_piece == 0) {
        this->h = 314159;
        for (int i = 1; i <= 6; i++) {
            this->h = std::min(
                this->h,
                this->kingDistance(this->pos[i], this->row * this->col - 1));
        }
    } else {
        this->h = this->kingDistance(this->pos[this->goal_piece],
                                     this->row * this->col - 1);
    }
}

int Game::kingDistance(int pos_a, int pos_b) {
    if (pos_a == -1 || pos_b == -1) {  // piece eaten, can't make it
        return 314159;
    }
    int x_a = pos_a % this->col, y_a = pos_a / this->col;
    int x_b = pos_b % this->col, y_b = pos_b / this->col;
    return std::max(std::abs(x_b - x_a), std::abs(y_b - y_a));
}

}  // namespace ewn
