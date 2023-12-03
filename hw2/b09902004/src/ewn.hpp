#ifndef EWN_HPP
#define EWN_HPP

#include <cstdint>

namespace ewn {

// Game environment
constexpr int ROW = 6;
constexpr int COL = 7;
constexpr int MAX_CUBES = 6;
constexpr int PERIOD = 21;  // the period of the given dice sequence
constexpr int RED = 0;
constexpr int BLUE = 1;

// Constraints (might be tunable)
constexpr int MAX_PLIES = 150;  // the average ply of a game is far smaller
constexpr int MAX_MOVES = 8;
constexpr int MAX_NODES = 1000000;

// Tunable parameters
constexpr double UCB_C = 1.18;  // Balancing exploitation and exploration
constexpr int SIM_BATCH = 25;
constexpr int SIM_THRES = 200;
constexpr double SEARCH_TIME = 1.8;

class State {
   private:
    static inline int s_dice_seq[PERIOD]{-1};  // shared among all
    int m_board[ROW * COL];
    int m_pos[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int m_num_cubes[2];
    int m_next;  // next player
    int m_n_plies;

   public:
    State();
    State(const State& rhs) = default;
    void init_board();
    bool is_over() const;
    int get_winner() const;
    int get_round_player() const;
    int move_gen_all(int* move_arr) const;
    void do_move(int move);
    int find_mate_in_1(int move_arr[], int n_moves) const;
    double get_score() const;

    void log_board();
};

inline int get_random_move(const State&);

class Node {
   private:
    static inline int s_id_generator{0};
    static inline int s_max_depth{0};
    static inline double s_avg_depth{0.0};

    State m_game_state;
    int m_ply;
    int m_id;
    int m_parent_id;
    int m_child_id[MAX_MOVES];
    int m_n_childs;
    int m_depth;
    bool m_is_terminal;
    // For calculating UCB
    int m_N;
    int m_W;
    double m_score;
    double m_win_rate;     // This is always win rate for *us*, regardless of turn player
    double m_avg_score;    // This is always score for *us*
    double m_sqrtN;        // sqrt(N)
    double m_c_sqrt_logN;  // c*sqrt(log(N))

   public:
    static Node* create_root(const State& game_state);
    static void log_stats();

    double win_rate() const;
    double UCB_score() const;
    Node* parent() const;
    Node* child(int idx) const;
    int ply() const;
    int n_childs() const;
    void expand();
    void simulate_and_backward();
    void update(int N, int W, double score);

    Node* find_PV_leaf();
    bool should_expand() const;
    void simulate_and_backward_children();
};

void reset_simulation_count();
void log_simulation_count();

}  // namespace ewn

#endif
