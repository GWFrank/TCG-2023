#include <bits/stdc++.h>
#include "ewn.hpp"

inline void PrintDummyAnswer() {
    printf("0\n");
}

// Returns whether a goal state is reached
bool DFSLimit(ewn::Game &game, int limit) {
    int moves[16]; // at most can move 2 pieces in 8 directions each
    int move_count = game.move_gen_all(moves);
    for (int idx = 0; idx < move_count; idx++) {
        game.do_move(moves[idx]);
        if (game.is_goal()) {
            return true;
        }
        if (limit > 1 && DFSLimit(game, limit-1)) {
            return true;
        }
        game.undo();
    }
    return false;
}

void DFID(ewn::Game &game) {
    int quit_limit = (ewn::COL-1) * (ewn::ROW-1) * 6;
    for (int limit = 1; limit < quit_limit; limit++) {
        bool found = DFSLimit(game, limit);
        if (found) {
            game.print_history();
            return;
        }
    }
    PrintDummyAnswer();
}

int main() {
    ewn::Game game;
    game.scan_board();
    DFID(game);

    return 0;
}