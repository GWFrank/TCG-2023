#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <iostream>

#include "ewn.hpp"

int recv_move(int enemy) {
    int num = getchar() - '0';
    int dir = getchar() - '0';
    if (enemy == ewn::BLUE) num += ewn::MAX_CUBES;
    return num << 4 | dir;
}

void send_move(int move) {
    int num = move >> 4;
    int dir = move & 0xf;
    if (num >= ewn::MAX_CUBES) num -= ewn::MAX_CUBES;
    printf("%d%d", num, dir);
    fflush(stdout);
}

int MCS(const ewn::State& current) {
    std::timespec ts_start, ts_now;
    std::timespec_get(&ts_start, TIME_UTC);

    static int move_arr[ewn::MAX_MOVES];
    int n_moves = current.move_gen_all(move_arr);
    if (n_moves == 1) {  // Shortcut only 1 legal move
        return move_arr[0];
    }
    int quick_mate = current.find_mate_in_1(move_arr, n_moves);
    if (quick_mate != -1) {  // Shortcut reaching goal in 1 move
        return quick_mate;
    }

    ewn::Node* root = ewn::Node::create_root(current);
    root->expand();
    int n_childs = root->n_childs();

    // Initial run
    // #ifndef NDEBUG
    //     std::cerr << "Starting initial simulation()\n";
    // #endif
    root->simulate_and_backward_children();

    // Find child with highest UCB score and simulate, until time runs out
    // #ifndef NDEBUG
    //     std::cerr << "Finding highest UCB and simulate\n";
    // #endif
    while (true) {
        timespec_get(&ts_now, TIME_UTC);
        double wall_clock_time = static_cast<double>(ts_now.tv_sec + ts_now.tv_nsec * 1e-9) -
                                 static_cast<double>(ts_start.tv_sec + ts_start.tv_nsec * 1e-9);
        if (wall_clock_time >= ewn::SEARCH_TIME) {
            break;
        }

        ewn::Node* best_ucb_child = root->child(0);
        double best_ucb_score = root->child(0)->UCB_score();
        for (int i = 1; i < n_childs; i++) {
            double score_i = root->child(i)->UCB_score();
            if (score_i > best_ucb_score) {
                best_ucb_child = root->child(i);
                best_ucb_score = score_i;
            }
        }

        best_ucb_child->simulate_and_backward();
    }

    // Select the child with highest win rate
    // #ifndef NDEBUG
    //     std::cerr << "Selecting best child\n";
    // #endif
    ewn::Node* best_wr_child = root->child(0);
    double best_wr = root->child(0)->win_rate();
#ifndef NDEBUG
    std::cerr << "Child 0 win rate: " << best_wr << "\n";
#endif
    for (int i = 1; i < n_childs; i++) {
        double wr_i = root->child(i)->win_rate();
#ifndef NDEBUG
        std::cerr << "Child " << i << " win rate: " << wr_i << "\n";
#endif
        if (wr_i > best_wr) {
            best_wr_child = root->child(i);
            best_wr = wr_i;
        }
    }

    return best_wr_child->ply();
}

int MCTS(const ewn::State& current) {
    std::timespec ts_start, ts_now;
    std::timespec_get(&ts_start, TIME_UTC);

    static int move_arr[ewn::MAX_MOVES];
    int n_moves = current.move_gen_all(move_arr);
    if (n_moves == 1) {  // Shortcut only 1 legal move
        return move_arr[0];
    }
    int quick_mate = current.find_mate_in_1(move_arr, n_moves);
    if (quick_mate != -1) {  // Shortcut reaching goal in 1 move
        return quick_mate;
    }

    ewn::Node* root = ewn::Node::create_root(current);
    root->expand();

    // Initial run
    root->simulate_and_backward_children();

    // Find PV's leaf
    while (true) {
        timespec_get(&ts_now, TIME_UTC);
        double wall_clock_time = static_cast<double>(ts_now.tv_sec + ts_now.tv_nsec * 1e-9) -
                                 static_cast<double>(ts_start.tv_sec + ts_start.tv_nsec * 1e-9);
        if (wall_clock_time >= ewn::SEARCH_TIME) {
            break;
        }

        ewn::Node* pv_leaf = root->find_PV_leaf();
        if (pv_leaf->should_expand()) {
            pv_leaf->expand();
            pv_leaf->simulate_and_backward_children();
        } else {
            pv_leaf->simulate_and_backward();
        }
    }

    // Select the child with highest win rate
    ewn::Node* best_wr_child = root->child(0);
    double best_wr = root->child(0)->win_rate();
#ifndef NDEBUG
    std::cerr << "Child 0 win rate: " << best_wr << "\n";
#endif
    for (int i = 1; i < root->n_childs(); i++) {
        double wr_i = root->child(i)->win_rate();
#ifndef NDEBUG
        std::cerr << "Child " << i << " win rate: " << wr_i << "\n";
#endif
        if (wr_i > best_wr) {
            best_wr_child = root->child(i);
            best_wr = wr_i;
        }
    }

    return best_wr_child->ply();
}

int main() {
    ewn::State game;
    bool my_turn;
    int enemy;
    int move;

    do {
        game.init_board();
        my_turn = getchar() == 'f';
        enemy = my_turn ? ewn::BLUE : ewn::RED;
        while (!game.is_over()) {
            if (!my_turn) {
                move = recv_move(enemy);
                game.do_move(move);
            } else {
#ifndef NDEBUG
                ewn::reset_simulation_count();
                std::cerr << "==== Enter MCS() ====\n";
                game.log_board();
#endif
                // move = MCS(game);
                move = MCTS(game);
#ifndef NDEBUG
                ewn::log_simulation_count();
                std::cerr << "Piece: " << (move >> 4) << " Direction: " << (move & 0xf) << "\n";
                std::cerr << "==== Exit MCS() ====\n";
#endif
                game.do_move(move);
                send_move(move);
            }
            my_turn = !my_turn;
        }
    } while (getchar() == 'y');
}
