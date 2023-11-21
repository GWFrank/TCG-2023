#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include "ewn.h"

#define INF_DIST 999
#define WIN 10000
#define LOSE -(WIN)

int shortest_distance(int location, int color) {
    if (location == -1) return INF_DIST;
    const int row = location / COL;
    const int col = location % COL;
    if (color == BLUE) return std::max(row, col);
    return std::max(ROW - row - 1, COL - col - 1);
}

int EWN::greedy() {
    int min_dist[2] = {INF_DIST, INF_DIST};
    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            int dist = shortest_distance(pos[i + offset], player);
            if (min_dist[player] > dist) min_dist[player] = dist;
        }
        offset += MAX_CUBES;
    }
    if (next == BLUE) return min_dist[RED] - min_dist[BLUE];
    return min_dist[BLUE] - min_dist[RED];
}

int search(EWN &game, int alpha, int beta, int depth) {
    int move_arr[MAX_MOVES];
    int num_moves;
    int val = LOSE;
    int tmp;

    if (game.is_over()) {
        if (game.num_cubes[RED] == 0) return game.next == BLUE ? WIN : LOSE;
        if (game.num_cubes[BLUE] == 0) return game.next == RED ? WIN : LOSE;
        if (game.board[0] >= MAX_CUBES) return game.next == BLUE ? WIN : LOSE;
        // if (is_red_cube(game.board[ROW * COL - 1]))
        return game.next == RED ? WIN : LOSE;
    }
    if (depth == 0) {
        return game.greedy();
    }

    num_moves = game.move_gen_all(move_arr);
    for (int i = 0; i < num_moves; i++) {
        game.do_move(move_arr[i]);
        tmp = -search(game, -beta, -std::max(val, alpha), depth - 1);
        game.undo();
        if (val < tmp) {
            val = tmp;
            if (val >= beta) break;
        }
    }

    return val;
}

int search_and_get_move(EWN &game, int depth) {
    int move_arr[MAX_MOVES];
    int num_moves;
    int best_index = 0;
    int val = LOSE;
    int tmp;

    num_moves = game.move_gen_all(move_arr);
    for (int i = 0; i < num_moves; i++) {
        game.do_move(move_arr[i]);
        tmp = -search(game, LOSE, -val, depth - 1);
        game.undo();
        if (val < tmp) {
            val = tmp;
            best_index = i;
            if (val >= WIN) break;
        }
    }
    if (val == LOSE) {
        for (int i = 0; i < num_moves; i++) {
            if ((move_arr[i] & 0xf) == 2) {
                best_index = i;
                break;
            }
        }
    }

    return move_arr[best_index];
}

int get_random_move(EWN &game) {
    int move_arr[MAX_MOVES];
    int num_moves = game.move_gen_all(move_arr);
    return move_arr[arc4random_uniform(num_moves)];
}
