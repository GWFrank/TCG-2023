#ifndef AGENT_H
#define AGENT_H

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <ctime>

#include <string>
#include <utility>

namespace ewn {

// Environment-related, don't touch
constexpr int k_red = 0;
constexpr int k_blue = 1;
constexpr int k_board_size = 5;
constexpr int k_piece_num = 6;
constexpr int k_command_num = 7;

// Magic numbers
constexpr int k_no_dice = 0;
constexpr int k_no_piece = 0;
constexpr int k_off_board = -1;
constexpr int k_blue_goal = 0;
constexpr int k_red_goal = k_board_size * k_board_size - 1;

constexpr int k_search_depth = 4;

// Algorithm-related
using score_t = double;
using move_t = int32_t;  // | unused | dice (4) | eaten piece (4) | moving piece (4) | direction (4) |
using move_score = std::pair<move_t, score_t>;  // First item is the move, second item is the score
constexpr score_t k_max_score = 100;
constexpr score_t k_min_score = -k_max_score;

constexpr int k_max_moves = 6;
constexpr int k_max_history = 50;

class Agent;
class State;

class Agent {
    const char* commands_name[k_command_num] = {"name", "version", "time_setting", "board_setting",
                                                "ini",  "get",     "exit"};

   public:
    Agent(void);
    ~Agent(void);

    // commands
    void Name(const char* data[], char* response);
    void Version(const char* data[], char* response);
    void Time_setting(const char* data[], char* response);
    void Board_setting(const char* data[], char* response);
    void Ini(const char* data[], char* response);
    void Get(const char* data[], char* response);
    void Exit(const char* data[], char* response);

   private:
    bool m_red_exist[k_piece_num], m_blue_exist[k_piece_num];
    int m_color;
    int m_red_time, m_blue_time;
    int m_board_size;
    int m_dice;
    int m_board[k_board_size][k_board_size];
    int m_red_piece_num, m_blue_piece_num;

    // Board
    void Init_board_state(char* position);
    void Set_board(char* position);
    void Print_chessboard();
    void Generate_move(char move[]);
    void Make_move(const int piece, const int start_point, const int end_point);
    int get_legal_move(int result[]);
    int referee(int piece, int* src, int* dst);

    // Game Algos
    move_score negascout(State& state, score_t alpha, score_t beta, int depth);
    move_score negascout_chance(State& state, score_t alpha, score_t beta, int depth);

    friend State;
};

class State {
   private:
    int m_pos[2 * k_piece_num + 1];  // 1-6 is blue, 7-12 is red
    int m_board[k_board_size * k_board_size];
    move_t m_history[k_max_history];

    int m_our_color;
    int m_round_color;
    int m_board_size;
    int m_dice;
    int m_red_piece_num, m_blue_piece_num;
    int m_history_len;

   public:
    State();
    State(const Agent* agent);
    State(const State& rhs) = default;
    bool is_chance_node() const;
    bool is_over() const;
    int winner() const;

    void do_move(move_t move);
    void undo();
    int move_gen_all(move_t move_arr[]) const;
    void set_dice(int dice);
    void unset_dice();
    score_t evaluate();  // + means good for round player, - means bad for round player

    void to_agent_move(move_t move, int agent_move[3]) const;

    void log_self() const;
    bool check_equal(const State& rhs) const;
};
}  // namespace ewn
#endif