#include <cstdio>
#include <cstdlib>
#include <cstring>
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
#ifdef HARD
                move = ewn::search_and_get_move(game, 8);
#elif defined(NORMAL)
                move = ewn::search_and_get_move(game, 2);
#elif defined(EASY)
                move = ewn::get_random_move(game);
#else
                fprintf(stderr, "Please define at least one mode.\n");
                exit(1);
#endif
                game.do_move(move);
                send_move(move);
            }
            my_turn = !my_turn;
        }
    } while (getchar() == 'y');
}
