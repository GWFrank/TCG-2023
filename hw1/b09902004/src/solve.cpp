#include <queue>
#include <vector>
#include <unordered_set>

#include <cstdio>
#include <cstdint>

#include "ewn.hpp"

inline void PrintDummyAnswer() { printf("0\n"); }

// Returns whether a goal state is reached
bool DFSLimit(ewn::Game &game, int limit) {
    int moves[16];  // at most can move 2 pieces in 8 directions each
    int move_count = game.generateAllMoves(moves);
    for (int idx = 0; idx < move_count; idx++) {
        game.doMove(moves[idx]);
        if (game.isGoal()) {
            return true;
        }
        if (limit > 1 && DFSLimit(game, limit - 1)) {
            return true;
        }
        game.undo();
    }
    return false;
}

void DFID(ewn::Game &game) {
    int quit_limit = (ewn::COL - 1) * (ewn::ROW - 1) * 6;
    for (int limit = 1; limit < quit_limit; limit++) {
        bool found = DFSLimit(game, limit);
        if (found) {
            game.printHistory();
            return;
        }
    }
    PrintDummyAnswer();
}

class CompareH1 {
   public:
    bool operator()(ewn::Game &game1, ewn::Game &game2) {
        return game1.currentCost() + game1.heuristic() >
               game2.currentCost() + game2.heuristic();
    }
};

void AStar(ewn::Game initial_state) {
    std::priority_queue<ewn::Game, std::vector<ewn::Game>, CompareH1> fringe;
    std::unordered_set<uint64_t> seen_states;

    fringe.push(initial_state);
    while (!fringe.empty()) {
        ewn::Game state = fringe.top();
        fringe.pop();

        if (state.isGoal()) {
            state.printHistory();
            return;
        }

        int moves[16];
        int move_count = state.generateAllMoves(moves);
        for (int idx = 0; idx < move_count; idx++) {
            ewn::Game next_state = state;
            next_state.doMove(moves[idx]);
            if (!next_state.isDoable()) {
                continue;
            }
            if (!state.isImproving(moves[idx])) {
                continue;
            }
            uint64_t next_state_hash = next_state.hash();
            if (seen_states.count(next_state_hash) != 0) {
                continue;
            }
            seen_states.insert(next_state_hash);
            fringe.push(next_state);
        }
    }

    // Return a dummy answer if AStar fails. This should not happen.
    PrintDummyAnswer();
}

int main() {
    ewn::Game game;
    game.scanBoard();
    // DFID(game);
    AStar(game);

    return 0;
}
