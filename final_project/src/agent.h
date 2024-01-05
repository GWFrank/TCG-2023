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

using score_t = double;
using move_t = int32_t;  // | unused | dice (4) | eaten piece (4) | moving piece (4) | direction (4) |
using move_score = std::pair<move_t, score_t>;  // First item is the move, second item is the score
using hash_t = uint64_t;
using score_flag_t = int;

// Environment-related, don't touch
constexpr int k_red = 0;
constexpr int k_blue = 1;
constexpr int k_board_size = 5;
constexpr int k_piece_num = 6;
constexpr int k_command_num = 7;

// Magic numbers
constexpr int k_no_dice = 0;
constexpr int k_no_piece = 0;
constexpr int k_off_board_pos = -1;
constexpr int k_blue_goal = 0;
constexpr int k_red_goal = k_board_size * k_board_size - 1;
constexpr score_t k_window_epsilon = 0.001;
constexpr score_t k_cmp_epsilon = 0.00001;
constexpr int k_rounding_decimals = 2;
constexpr move_t k_null_move = -1;
constexpr int k_star0 = 0;
constexpr int k_star1 = 1;

// Algorithm-related
constexpr int k_start_depth = 6, k_max_depth = 24;
constexpr double k_total_time = 50;
constexpr double k_time_per_move = 15;
constexpr score_t k_max_score = 100;
constexpr score_t k_min_score = -k_max_score;

constexpr int k_max_moves = 6;
constexpr int k_max_history = 50;

class Agent;
class State;
struct tt_entry;

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
    move_score iterative_deepening(State& state) const;
    move_score negascout(State& state, score_t alpha, score_t beta, int depth) const;
    move_score negascout_chance(State& state, score_t alpha, score_t beta, int depth, int mode) const;

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

    hash_t m_zobrist_hash;

    move_t find_mate(int move_arr[], int n_moves) const;
    move_t defend_mate(int move_arr[], int n_moves) const;

    score_t distance_h() const;
    score_t determinacy_h() const;

   public:
    State();
    State(const Agent* agent);
    State(const State& rhs) = default;
    bool is_chance_node() const;
    bool is_over() const;
    int winner() const;

    int get_round_color() const;
    hash_t get_hash_value() const;

    void do_move(move_t move);
    void undo();
    int move_gen_all(move_t move_arr[]) const;
    void set_dice(int dice);
    void unset_dice();
    score_t evaluate() const;  // + means good for round player, - means bad for round player

    void to_agent_move(move_t move, int agent_move[3]) const;

    void log_self() const;
    bool check_equal(const State& rhs) const;

    move_t shortcut() const;
};

constexpr int tt_size_bits = 17;
constexpr hash_t tt_index_mask = (1 << tt_size_bits) - 1;
constexpr score_flag_t k_is_exact = 0;
constexpr score_flag_t k_is_upper_bound = 1;
constexpr score_flag_t k_is_lower_bound = 2;

struct tt_entry {
    bool valid{false};
    hash_t hash_key{0};
    score_t position_score{0};
    int depth{-1};
    score_flag_t score_type{k_is_exact};
};

}  // namespace ewn
#endif