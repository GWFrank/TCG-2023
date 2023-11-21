#ifndef EWN_H
#define EWN_H

#define ROW 6
#define COL 7
#define MAX_CUBES 6
#define RED 0
#define BLUE 1

#define PERIOD 21      // the period of the given dice sequence
#define MAX_PLIES 150  // the average ply of a game is far smaller than this number
#define MAX_MOVES 8

class EWN {
    int board[ROW * COL];
    int pos[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int num_cubes[2];
    int next;  // next player

    int dice_seq[PERIOD];
    int history[MAX_PLIES];
    int n_plies;

public:
    void init_board();
    bool is_over();
    int move_gen_all(int *move_arr);
    void do_move(int move);
    void undo();

    int greedy();
    // int heuristic();
    friend int search(EWN &, int alpha, int beta, int depth);
};

int search_and_get_move(EWN &, int depth);
int get_random_move(EWN &);

#endif
