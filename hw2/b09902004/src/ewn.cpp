#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>

#include <iostream>

#include "ewn.hpp"

namespace ewn {

inline bool is_red_cube(int x) { return (x >= 0) && (x < MAX_CUBES); }

inline bool is_blue_cube(int x) { return (x >= MAX_CUBES) && (x < (MAX_CUBES << 1)); }

inline bool is_red_cube_fast(int x) { return x < MAX_CUBES; }

inline bool is_blue_cube_fast(int x) { return x >= MAX_CUBES; }

inline bool is_empty_cube(int x) { return x == 0xf; }

inline void change_player(int &x) { x ^= 1; }

// clang-format off
// -COL-1 | -COL | -COL+1
//     -1 |   0  |      1
//  COL-1 |  COL |  COL+1
static const int dir_val[2][4] = {
    { 1,  COL,  COL + 1, -COL + 1},
    {-1, -COL, -COL - 1,  COL - 1}
};

static const int init_pos[2][MAX_CUBES] = {
    {              0,               1,               2,         COL,     COL + 1,     COL * 2},
    {(ROW-2)*COL - 1, (ROW-1)*COL - 2, (ROW-1)*COL - 1, ROW*COL - 3, ROW*COL - 2, ROW*COL - 1}
};
// clang-format on

// Globals
static Node All_Nodes[MAX_NODES];
static int64_t total_simulation = 0;

State::State() {
    std::memset(m_board, 0x00, sizeof(m_board));
    std::memset(m_pos, 0x00, sizeof(m_pos));
    m_num_cubes[0] = MAX_CUBES;
    m_num_cubes[0] = MAX_CUBES;
    m_next = RED;
    m_n_plies = 0;
}

void State::init_board() {
    std::memset(m_board, 0xff, sizeof(m_board));
    std::memset(m_pos, 0xff, sizeof(m_pos));
    m_num_cubes[0] = MAX_CUBES;
    m_num_cubes[1] = MAX_CUBES;
    m_next = RED;
    m_n_plies = 0;

    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            int cube = getchar() - '0';
            m_board[init_pos[player][i]] = cube + offset;
            m_pos[cube + offset] = init_pos[player][i];
        }
        offset += MAX_CUBES;
    }
    for (int i = 0; i < PERIOD; i++) {
        State::s_dice_seq[i] = getchar() - '0';
    }
}

bool State::is_over() const {
    if (m_num_cubes[0] == 0 || m_num_cubes[1] == 0) return true;
    if (is_blue_cube(m_board[0])) return true;
    if (is_red_cube(m_board[ROW * COL - 1])) return true;
    return false;
}

// Return either ewn::BLUE or ewn::RED
int State::get_winner() const {
#ifndef NDEBUG
    assert(is_over());
#endif
    if (m_num_cubes[RED] == 0 || is_blue_cube_fast(m_board[0])) {
        return BLUE;
    } else {
        return RED;
    }
}

int State::get_round_player() const { return m_next; }

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the cube number
11~8: store the eaten cube (used only in history)
*/

int move_gen(int *move_arr, int cube, int location) {
    int count = 0;
    const int row = location / COL;
    const int col = location % COL;
    bool h_ok;      // horizontal
    bool v_ok;      // vertical
    bool rev_v_ok;  // reverse, vertical

    if (is_red_cube_fast(cube)) {
        h_ok = col != COL - 1;
        v_ok = row != ROW - 1;
        rev_v_ok = row != 0;
    } else {
        h_ok = col != 0;
        v_ok = row != 0;
        rev_v_ok = row != ROW - 1;
    }
    if (h_ok) move_arr[count++] = cube << 4;
    if (v_ok) move_arr[count++] = cube << 4 | 1;
    if (h_ok && v_ok) move_arr[count++] = cube << 4 | 2;
    if (h_ok && rev_v_ok) move_arr[count++] = cube << 4 | 3;

    return count;
}

int State::move_gen_all(int *move_arr) const {
    int count = 0;
    const int dice = State::s_dice_seq[m_n_plies % PERIOD];
    const int offset = m_next == BLUE ? MAX_CUBES : 0;
    const int *self_pos = m_pos + offset;

    if (self_pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (small >= 0 && self_pos[small] == -1) small--;
        while (large < MAX_CUBES && self_pos[large] == -1) large++;

        if (small >= 0) {
            count += move_gen(move_arr, small + offset, self_pos[small]);
        }
        if (large < MAX_CUBES) {
            count += move_gen(move_arr + count, large + offset, self_pos[large]);
        }
    } else {
        count = move_gen(move_arr, dice + offset, self_pos[dice]);
    }

    return count;
}

void State::do_move(int move) {
    int cube = move >> 4;
    int direction = move & 0xf;
    int dst = m_pos[cube] + dir_val[m_next][direction];

    if (m_n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (m_board[dst] >= 0) {
        if (is_red_cube_fast(m_board[dst])) {
            m_num_cubes[RED]--;
        } else {
            m_num_cubes[BLUE]--;
        }
        m_pos[m_board[dst]] = -1;
        move |= m_board[dst] << 8;
    } else {
        move |= 0xf00;
    }
    m_board[m_pos[cube]] = -1;
    m_board[dst] = cube;
    m_pos[cube] = dst;
    m_n_plies++;
    change_player(m_next);
}

int State::find_mate_in_1(int move_arr[], int n_moves) const {
    for (int i = 0; i < n_moves; i++) {
        int move = move_arr[i];
        int cube = move >> 4;
        int direction = move & 0xf;
        int dst = m_pos[cube] + dir_val[m_next][direction];
        if (is_red_cube_fast(cube)) {
            if (dst == ROW * COL - 1) {
                return move;
            }
            if (m_num_cubes[BLUE] == 1 && is_blue_cube_fast(m_board[dst])) {
                return move;
            }
        } else if (is_blue_cube_fast(cube)) {
            if (dst == 0) {
                return move;
            }
            if (m_num_cubes[RED] == 1 && is_red_cube_fast(m_board[dst])) {
                return move;
            }
        }
    }
    return -1;
}

void State::log_board() {
    std::cerr << "Next dice: " << s_dice_seq[m_n_plies % PERIOD] << "\n";

    for (int r = 0; r < ROW; r++) {
        for (int c = 0; c < COL; c++) {
            int piece = m_board[r * COL + c];
            if (piece != -1) {
                fprintf(stderr, "%02d ", piece);
            } else {
                fprintf(stderr, "__ ");
            }
        }
        std::cerr << "\n";
    }
}

double State::get_score() const {
    double score = 0;
    constexpr double max_score = 10;

    if (m_num_cubes[RED] == 0 || m_num_cubes[BLUE] == 0) {
        score += 10;
    } else {
        score += 9;
    }

    score /= max_score;

    return score;
    // return 1;
}

Node *Node::create_root(const State &game_state) {
    Node &root = All_Nodes[0];
    s_id_generator = 0;
    root = Node{};

    root.m_game_state = game_state;
    root.m_ply = -1;
    root.m_id = s_id_generator;
    root.m_parent_id = -1;
    root.m_n_childs = 0;
    root.m_depth = 0;
    root.m_is_terminal = root.m_game_state.is_over();

    root.m_N = 0;
    root.m_W = 0;
    root.m_score = 0.0;
    root.m_win_rate = 0.0;
    root.m_avg_score = 0.0;
    root.m_sqrtN = 0.0;
    root.m_c_sqrt_logN = 0.0;

    s_id_generator++;

#ifndef NDEBUG
    s_avg_depth = 0.0;
    s_max_depth = 0;
#endif
    return &root;
}

void Node::log_stats() {
    std::cerr << "Total nodes: " << s_id_generator << "\n";
    std::cerr << "Max depth: " << s_max_depth << "\n";
    std::cerr << "Average depth: " << s_avg_depth << "\n";
}

double Node::win_rate() const { return m_win_rate; }

double Node::UCB_score() const {
    // double exploitation = m_win_rate;
    double exploitation = m_avg_score;
    // #ifndef NDEBUG
    //     assert(exploitation <= 1);
    //     assert(exploitation >= 0);
    //     std::cerr << m_win_rate << ", " << m_avg_score << "\n";
    // #endif

    bool seen_from_min_node = (m_depth % 2 == 0);
    if (seen_from_min_node) {
        exploitation = 1 - exploitation;
    }
    double exploration = All_Nodes[m_parent_id].m_c_sqrt_logN / m_sqrtN;
    return exploitation + exploration;
}

Node *Node::parent() const {
    if (m_parent_id == -1) {
        return nullptr;
    }
    return &All_Nodes[m_parent_id];
}

Node *Node::child(int idx) const { return &(All_Nodes[m_child_id[idx]]); }

int Node::ply() const { return m_ply; }

int Node::n_childs() const { return m_n_childs; }

void Node::expand() {
    int move_arr[MAX_MOVES];
    int num_moves = m_game_state.move_gen_all(move_arr);
    for (int i = 0; i < num_moves; i++) {
        int cid = s_id_generator;
        s_id_generator++;
#ifndef NDEBUG
        assert(s_id_generator < MAX_NODES);
#endif
        m_child_id[m_n_childs] = cid;
        m_n_childs++;

        Node &child = All_Nodes[cid];
        child.m_game_state = m_game_state;
        child.m_game_state.do_move(move_arr[i]);
        child.m_ply = move_arr[i];
        child.m_id = cid;
        child.m_parent_id = m_id;
        child.m_n_childs = 0;
        child.m_depth = m_depth + 1;
        child.m_is_terminal = child.m_game_state.is_over();

        child.m_N = 0;
        child.m_W = 0;
        child.m_score = 0.0;
        child.m_win_rate = 0.0;
        child.m_avg_score = 0.0;
        child.m_sqrtN = 0.0;
        child.m_c_sqrt_logN = 0.0;
#ifndef NDEBUG
        s_max_depth = std::max(s_max_depth, m_depth + 1);
        s_avg_depth = (s_avg_depth * (s_id_generator - 1) + (static_cast<double>(m_depth) + 1)) /
                      static_cast<double>(s_id_generator);
#endif
    }
}

// Simulate SIM_BATCH times and back-propagate the result
void Node::simulate_and_backward() {
    int wins = 0;
    double total_score = 0;
    for (int i = 0; i < SIM_BATCH; i++) {
        State sim_state{m_game_state};
        int move_arr[MAX_MOVES];
        while (!sim_state.is_over()) {
            // TODO: add shortcuts
            int n_moves = sim_state.move_gen_all(move_arr);
            if (n_moves == 1) {  // Shortcut only 1 legal move
                sim_state.do_move(move_arr[0]);
                continue;
            }
            int quick_mate = sim_state.find_mate_in_1(move_arr, n_moves);
            if (quick_mate != -1) {  // Shortcut reaching goal in 1 move
                sim_state.do_move(quick_mate);
                // #ifndef NDEBUG
                //                 std::cerr << "Shortcut during sim!\n";
                // #endif
                continue;
            }
            sim_state.do_move(move_arr[arc4random_uniform(n_moves)]);
        }
        bool is_max_node = (m_depth % 2 == 0);
        if ((sim_state.get_winner() == m_game_state.get_round_player()) == is_max_node) {
            // When "we" wins
            wins++;
            total_score += sim_state.get_score();
        } else {
            total_score += 1 - sim_state.get_score();
        }
    }
#ifndef NDEBUG
    total_simulation += SIM_BATCH;
#endif

    Node *cur_p = this;
    while (cur_p != nullptr) {
        cur_p->update(SIM_BATCH, wins, total_score);
        cur_p = cur_p->parent();
    }
}

void Node::update(int N, int W, double score) {
    m_N += N;
    m_W += W;
    m_score += score;

    m_win_rate = static_cast<double>(m_W) / static_cast<double>(m_N);
    m_avg_score = static_cast<double>(m_score) / static_cast<double>(m_N);
    m_sqrtN = std::sqrt(m_N);
    m_c_sqrt_logN = UCB_C * std::sqrt(std::log(m_N));
}

Node *Node::find_PV_leaf() {
    Node *pv_node = this;
    while (pv_node->m_n_childs != 0) {
        Node *best_ucb_child = pv_node->child(0);
        double best_ucb_score = pv_node->child(0)->UCB_score();
        for (int i = 1; i < pv_node->m_n_childs; i++) {
            double score_i = pv_node->child(i)->UCB_score();
            if (score_i > best_ucb_score) {
                best_ucb_child = pv_node->child(i);
                best_ucb_score = score_i;
            }
        }
        pv_node = best_ucb_child;
    }
    return pv_node;
}

bool Node::should_expand() const { return !m_is_terminal && m_N >= SIM_THRES; }

void Node::simulate_and_backward_children() {
    for (int i = 0; i < m_n_childs; i++) {
        child(i)->simulate_and_backward();
    }
}

void reset_simulation_count() { total_simulation = 0; }

void log_simulation_count() { std::cerr << "Total " << total_simulation << " simulations\n"; }

}  // namespace ewn
