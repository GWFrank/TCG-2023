// Game-logic in ewn::State is mainly modified from sample code in HWK2

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <algorithm>
#include <iostream>

#include "agent.h"
#include "precompute.h"

namespace ewn {

// Blue is 1-6, red is 7-12 (following the provided template)
inline bool is_red_piece(int x) { return x > k_piece_num && x <= 2 * k_piece_num; }
inline bool is_blue_piece(int x) { return x > k_no_piece && x <= k_piece_num; }

inline int next_player(int x) {
    if (x == k_red) return k_blue;
    if (x == k_blue) return k_red;
    fprintf(stderr, "Unexpected player color %d", x);
    exit(1);
}

inline score_t round_up(score_t score) {
    const double multiplier = std::pow(10.0, k_rounding_decimals);
    return std::round(score * multiplier) / multiplier;
}

Agent::Agent(void) {}

Agent::~Agent(void) {}

void Agent::Name(const char* data[] __attribute__((unused)), char* response) {
#ifdef RANDOM_MOVE
    strcpy(response, "Random Agent");
#else
    strcpy(response, "Smort Agent");
#endif
}

void Agent::Version(const char* data[] __attribute__((unused)), char* response) { strcpy(response, "0.0.0"); }

void Agent::Time_setting(const char* data[], char* response) {
    this->m_red_time = std::stoi(data[1]);
    this->m_blue_time = std::stoi(data[1]);
    strcpy(response, "1");
}

void Agent::Board_setting(const char* data[], char* response) {
    this->m_board_size = std::stoi(data[1]);
    this->m_red_piece_num = std::stoi(data[2]);
    this->m_blue_piece_num = std::stoi(data[2]);
    strcpy(response, "1");
}

void Agent::Ini(const char* data[], char* response) {
    // set color
    if (!strcmp(data[1], "R")) {
        this->m_color = k_red;
    } else if (!strcmp(data[1], "B")) {
        this->m_color = k_blue;
    }

    char position[15];
    this->Init_board_state(position);

    sprintf(response, "%c%c %c%c %c%c %c%c %c%c %c%c", position[0], position[1], position[2], position[3], position[4],
            position[5], position[6], position[7], position[8], position[9], position[10], position[11]);
}

void Agent::Get(const char* data[], char* response) {
    // set color
    if (!strcmp(data[1], "R")) {
        this->m_color = k_red;
    } else if (!strcmp(data[1], "B")) {
        this->m_color = k_blue;
    }

    // set dice & board
    this->m_dice = std::stoi(data[2]);
    char position[25];
    sprintf(position, "%s%s%s%s%s%s%s%s%s%s%s%s", data[3], data[4], data[5], data[6], data[7], data[8], data[9],
            data[10], data[11], data[12], data[13], data[14]);
    this->Set_board(position);

    // generate move
    char move[4];
    this->Generate_move(move);
    sprintf(response, "%c%c %c%c", move[0], move[1], move[2], move[3]);
}

void Agent::Exit(const char* data[] __attribute__((unused)), char* response __attribute__((unused))) {
    fprintf(stderr, "Bye~\n");
}

// *********************** AI FUNCTION *********************** //

void Agent::Init_board_state(char* position) {
    int order[k_piece_num] = {0, 1, 2, 3, 4, 5};
    std::string red_init_position = "A1B1C1A2B2A3";
    std::string blue_init_position = "E3D4E4C5D5E5";

    // assign the initial positions of your pieces in random order
    for (int i = 0; i < k_piece_num; i++) {
        int j = rand() % (k_piece_num - i) + i;
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    for (int i = 0; i < k_piece_num; i++) {
        if (this->m_color == k_red) {
            position[order[i] * 2] = red_init_position[i * 2];
            position[order[i] * 2 + 1] = red_init_position[i * 2 + 1];
        } else if (this->m_color == k_blue) {
            position[order[i] * 2] = blue_init_position[i * 2];
            position[order[i] * 2 + 1] = blue_init_position[i * 2 + 1];
        }
    }
}

void Agent::Set_board(char* position) {
    memset(this->m_board, 0, sizeof(this->m_board));
    memset(this->m_blue_exist, 1, sizeof(this->m_blue_exist));
    memset(this->m_red_exist, 1, sizeof(this->m_red_exist));
    this->m_blue_piece_num = k_piece_num;
    this->m_red_piece_num = k_piece_num;

    int lost_piece_num = 0;
    for (int i = 0; i < k_piece_num * 2; i++) {
        int index = i * 2 - lost_piece_num;

        // the piece does not exist
        while (position[index] == '0') {
            index = i * 2 - lost_piece_num + 1;
            lost_piece_num++;
            // blue
            if (i < k_piece_num) {
                this->m_blue_piece_num--;
                this->m_blue_exist[i] = 0;
            }
            // red
            else {
                this->m_red_piece_num--;
                this->m_red_exist[i - k_piece_num] = 0;
            }
            i += 1;
        }
        // 1~6: blue pieces; 7~12: red pieces
        if (i < k_piece_num * 2) {
            this->m_board[position[index + 1] - '1'][position[index] - 'A'] = i + 1;
        }
    }
    fprintf(stderr, "\nThe current board:\n");
    this->Print_chessboard();
}

void Agent::Print_chessboard() {
    fprintf(stderr, "\n");
    // 1~6 represent blue pieces; A~F represent red pieces
    for (int i = 0; i < k_board_size; i++) {
        fprintf(stderr, "<%d>   ", i + 1);
        for (int j = 0; j < k_board_size; j++) {
            if (this->m_board[i][j] <= k_piece_num)
                fprintf(stderr, "%d  ", this->m_board[i][j]);
            else
                fprintf(stderr, "%c  ", 'A' + (this->m_board[i][j] - 7));
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     ");
    for (int i = 0; i < k_board_size; i++) {
        fprintf(stderr, "<%c>", 'A' + i);
    }
    fprintf(stderr, "\n\n");
    fprintf(stderr, "The number of blue pieces: %d\nThe number of red pieces: %d\n", this->m_blue_piece_num,
            this->m_red_piece_num);
}

void Agent::Generate_move(char move[]) {
#ifdef RANDOM_MOVE
    int result[100];
    // get legal moves
    int move_count = this->get_legal_move(result);
    // randomly choose a legal move
    int rand_move = rand() % move_count;
    int piece = result[rand_move * 3];
    int start_point = result[rand_move * 3 + 1];
    int end_point = result[rand_move * 3 + 2];
#else
    State current_state = State(this);
    move_t move_found = k_null_move;
#ifndef NDEBUG
    score_t expected_score = 0;
    current_state.log_self();
#endif

    move_t shortcut_move = current_state.shortcut();
    if (shortcut_move == k_null_move) {
        move_score search_result = negascout(current_state, k_min_score - 1, k_max_score + 1, k_search_depth);
        // #ifndef NDEBUG
        //     fprintf(stderr, "After searching...\n");
        //     current_state.log_self();
        // #endif
        move_found = search_result.first;
#ifndef NDEBUG
        expected_score = search_result.second;
#endif
    } else {
        move_found = shortcut_move;
#ifndef NDEBUG
        expected_score = 2 * k_max_score;
#endif
    }

    int agent_move[3] = {-1};
    current_state.to_agent_move(move_found, agent_move);
    int piece = agent_move[0];
    int start_point = agent_move[1];
    int end_point = agent_move[2];
#ifndef NDEBUG
    fprintf(stderr, "Move: %d\nExpected score: %f\n", move_found, expected_score);
    fprintf(stderr, "Piece: %d\n", piece);
    fprintf(stderr, "Start point: %d\nEnd point: %d\n", start_point, end_point);
#endif

    // get random legal moves
    // int result[100];
    // int move_count = this->get_legal_move(result);
    // int rand_move = rand() % move_count;
    // int piece = result[rand_move * 3];
    // int start_point = result[rand_move * 3 + 1];
    // int end_point = result[rand_move * 3 + 2];
#endif
    // print the result
    fprintf(stderr, "============================\nMy result:\n");
    sprintf(move, "%c%c%c%c", 'A' + start_point % k_board_size, '1' + start_point / k_board_size,
            'A' + end_point % k_board_size, '1' + end_point / k_board_size);
    if (piece <= k_piece_num)
        fprintf(stderr, "Blue piece %d: (%c%c) -> (%c%c)\n", piece, move[0], move[1], move[2], move[3]);
    else
        fprintf(stderr, "Red piece %d: (%c%c) -> (%c%c)\n", piece - k_piece_num, move[0], move[1], move[2], move[3]);

    this->Make_move(piece, start_point, end_point);

    this->Print_chessboard();
    fprintf(stderr, "============================\n");
}

// get all legal moves
int Agent::get_legal_move(int result[]) {
    int src = -1, dst[3] = {-1};
    int movable_piece;
    int move_count = 0;
    int result_count = 0;

    if (this->m_color == k_blue) {
        // the corresponding piece is alive
        if (this->m_blue_exist[this->m_dice - 1]) {
            movable_piece = this->m_dice;
            move_count = this->referee(movable_piece, &src, dst);
            for (int i = result_count; i < result_count + move_count; i++) {
                result[i * 3] = movable_piece;
                result[i * 3 + 1] = src;
                result[i * 3 + 2] = dst[i];
            }
            result_count += move_count;
        }
        // the corresponding piece does not exist
        else {
            // seeking for the next-higher piece
            for (int i = this->m_dice; i <= k_piece_num; ++i) {
                if (this->m_blue_exist[i - 1]) {
                    movable_piece = i;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
            // seeking for the next-lower piece
            for (int i = this->m_dice; i >= 1; --i) {
                if (this->m_blue_exist[i - 1]) {
                    movable_piece = i;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
        }
    }

    else if (this->m_color == k_red) {
        // the corresponding piece is alive
        if (this->m_red_exist[this->m_dice - 1]) {
            movable_piece = this->m_dice + k_piece_num;
            move_count = this->referee(movable_piece, &src, dst);
            for (int i = result_count; i < result_count + move_count; i++) {
                result[i * 3] = movable_piece;
                result[i * 3 + 1] = src;
                result[i * 3 + 2] = dst[i];
            }
            result_count += move_count;
        }
        // the corresponding piece does not exist
        else {
            // seeking for the next-higher piece
            for (int i = this->m_dice; i <= k_piece_num; ++i) {
                if (this->m_red_exist[i - 1]) {
                    movable_piece = i + k_piece_num;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
            // seeking for the next-lower piece
            for (int i = this->m_dice; i >= 1; --i) {
                if (this->m_red_exist[i - 1]) {
                    movable_piece = i + k_piece_num;
                    move_count = this->referee(movable_piece, &src, dst);
                    int index = 0;
                    for (int j = result_count; j < result_count + move_count; j++, index++) {
                        result[j * 3] = movable_piece;
                        result[j * 3 + 1] = src;
                        result[j * 3 + 2] = dst[index];
                    }
                    result_count += move_count;
                    break;
                }
            }
        }
    }
    return result_count;
}

// get possible moves of the piece
int Agent::referee(int piece, int* src, int* dst) {
    for (int i = 0; i < k_board_size; i++) {
        for (int j = 0; j < k_board_size; j++) {
            if (this->m_board[i][j] == piece) {
                *src = i * k_board_size + j;
            }
        }
    }
    // blue piece
    if (piece <= k_piece_num) {
        // the piece is on the leftmost column
        if (*src % k_board_size == 0) {
            dst[0] = *src - k_board_size;  // up
            return 1;
        }
        // the piece is on the uppermost row
        else if (*src < k_board_size) {
            dst[0] = *src - 1;  // left
            return 1;
        } else {
            dst[0] = *src - 1;                 // left
            dst[1] = *src - k_board_size;      // up
            dst[2] = *src - k_board_size - 1;  // upper left
            return 3;
        }
    }

    // red piece
    else {
        // the piece is on the rightmost column
        if (*src % k_board_size == 4) {
            dst[0] = *src + k_board_size;  // down
            return 1;
        }
        // the piece is on the downmost row
        else if (*src >= k_board_size * (k_board_size - 1)) {
            dst[0] = *src + 1;  // right
            return 1;
        } else {
            dst[0] = *src + 1;                 // right
            dst[1] = *src + k_board_size;      // down
            dst[2] = *src + k_board_size + 1;  // bottom right
            return 3;
        }
    }
}

void Agent::Make_move(const int piece, const int start_point, const int end_point) {
    int start_row = start_point / k_board_size;
    int start_col = start_point % k_board_size;
    int end_row = end_point / k_board_size;
    int end_col = end_point % k_board_size;

    this->m_board[start_row][start_col] = 0;

    // there has another piece on the target sqaure
    if (this->m_board[end_row][end_col] > 0) {
        if (this->m_board[end_row][end_col] <= k_piece_num) {
            this->m_blue_exist[this->m_board[end_row][end_col] - 1] = 0;
            this->m_blue_piece_num--;
        } else {
            this->m_red_exist[this->m_board[end_row][end_col] - 7] = 0;
            this->m_red_piece_num--;
        }
    }
    this->m_board[end_row][end_col] = piece;
}

move_score Agent::negascout(State& state, score_t alpha, score_t beta, int depth) {
    move_score result;
    result.first = -1;
    if (state.is_over() || depth == 0) {
        result.second = state.evaluate();
        return result;
    }

    if (state.is_chance_node()) {
        // Call Star0/Star0.5/Star1
        return negascout_chance(state, alpha, beta, depth - 1);
    } else {
        // Regular NegaScout
        score_t lo_bound = k_min_score;
        score_t hi_bound = beta;
        move_score tmp_ret;
        score_t tmp;
        move_t move_arr[k_max_moves];
        int num_moves = state.move_gen_all(move_arr);
        // #ifndef NDEBUG
        //         const State og_state(state);
        // #endif

        // In case all moves are losing.
        result.first = move_arr[0];
        result.second = lo_bound;

        for (int i = 0; i < num_moves; i++) {
            state.do_move(move_arr[i]);
            // #ifndef NDEBUG
            //             State after_move(state);
            // #endif

            tmp_ret = negascout(state, -hi_bound, -std::max(alpha, lo_bound), depth - 1);

            tmp = -tmp_ret.second;
            // #ifndef NDEBUG
            //             // state.log_self();
            //             // fprintf(stderr, "evaluation: %f\n", state.evaluate());
            //             if (depth == k_search_depth) {
            //                 fprintf(stderr, "alpha: %f, beta: %f\n", alpha, beta);
            //                 fprintf(stderr, "lo_bound: %f hi_bound: %f tmp score: %f\n", lo_bound, hi_bound, tmp);
            //             }
            // #endif
            if (tmp > lo_bound) {
                if (hi_bound == beta || depth < 3 || tmp >= beta) {
                    lo_bound = tmp;
                } else {
                    tmp_ret = negascout(state, -beta, -tmp, depth - 1);
                    lo_bound = -tmp_ret.second;
                }
                result.first = move_arr[i];
                result.second = lo_bound;
                // #ifndef NDEBUG
                //                 if (depth == k_search_depth) {
                //                     fprintf(stderr, "assign result, move=%d, score=%f\n", result.first,
                //                     result.second);
                //                 }
                // #endif
            }
            state.undo();
            // #ifndef NDEBUG
            //             bool consistent = og_state.check_equal(state);
            //             if (!consistent) {
            //                 fprintf(stderr, "state after move:\n");
            //                 after_move.log_self();
            //                 fprintf(stderr, "move: %d\n", move_arr[i]);
            //                 exit(1);
            //             }
            // #endif
            if (lo_bound >= k_max_score || lo_bound >= beta) {
                return result;
            }
            hi_bound = std::max(alpha, lo_bound) + k_epsilon;  // for null-window
        }
        return result;
    }
}

move_score Agent::negascout_chance(State& state, score_t alpha, score_t beta, int depth) {
    move_score result, tmp_ret;
    result.first = -1;
    result.second = 0;
    const int total_choice = k_piece_num;
    score_t A = total_choice * (alpha - k_max_score) + k_max_score;
    score_t B = total_choice * (beta - k_min_score) + k_min_score;
    score_t lo_bound = k_min_score, hi_bound = k_max_score;

    for (int dice_roll = 1; dice_roll <= total_choice; dice_roll++) {
        state.set_dice(dice_roll);
        tmp_ret = negascout(state, std::max(A, k_min_score), std::min(B, k_max_score), depth);

        score_t v_i = tmp_ret.second;
        lo_bound = lo_bound + (v_i - lo_bound) / total_choice;
        hi_bound = hi_bound + (v_i - hi_bound) / total_choice;
        if (v_i >= B) {
            result.second = lo_bound;
            goto negascout_chance_return;
        }
        if (v_i <= A) {
            result.second = hi_bound;
            goto negascout_chance_return;
        }
        result.second += v_i;
        A = A + k_max_score - v_i;
        B = B + k_min_score - v_i;
        state.unset_dice();
    }
    result.second = round_up(result.second / total_choice);
negascout_chance_return:
    return result;
}

State::State() {}

State::State(const Agent* agent) {
    m_our_color = agent->m_color;
    m_round_color = agent->m_color;
    m_board_size = agent->m_board_size;
    m_dice = agent->m_dice;
    m_red_piece_num = agent->m_red_piece_num;
    m_blue_piece_num = agent->m_blue_piece_num;
    m_history_len = 0;

    for (int i = 0; i <= 2 * k_piece_num; i++) {
        m_pos[i] = k_off_board_pos;
    }

    for (int row = 0; row < m_board_size; row++) {
        for (int col = 0; col < m_board_size; col++) {
            int piece = agent->m_board[row][col];
            m_board[row * m_board_size + col] = piece;
            if (piece == k_no_piece) continue;
#ifndef NDEBUG
            assert(piece <= 2 * k_piece_num && piece > 0);
#endif
            m_pos[piece] = row * m_board_size + col;
        }
    }
}

bool State::is_chance_node() const { return m_dice == k_no_dice; }

bool State::is_over() const {
    if (m_red_piece_num == 0 || m_blue_piece_num == 0) return true;
    if (is_blue_piece(m_board[k_blue_goal])) return true;
    if (is_red_piece(m_board[k_red_goal])) return true;
    return false;
}

int State::winner() const {
#ifndef NDEBUG
    assert(is_over());
#endif
    if (m_red_piece_num == 0 || is_blue_piece(m_board[k_blue_goal])) {
        return true;
    } else {
        return false;
    }
}

constexpr int k_right = 1, k_left = -1, k_up = -k_board_size, k_down = k_board_size;
static const int dir_val[2][3] = {
    {k_right, k_down, k_right + k_down},  // red
    {k_left, k_up, k_left + k_up},        // blue
};

void State::do_move(move_t move) {
    int direction = move & 0xf;
    int piece = (move >> 4) & 0xf;
    int dst = m_pos[piece] + dir_val[m_round_color][direction];
    int piece_on_dst = m_board[dst];

    if (m_history_len >= k_max_history) {
        fprintf(stderr, "Maximum history len exceeded, aborting.\n");
        exit(1);
    }

    if (piece_on_dst != k_no_piece) {
        if (is_red_piece(piece_on_dst)) {
            m_red_piece_num--;
        } else {
            m_blue_piece_num--;
        }
        m_pos[piece_on_dst] = k_off_board_pos;
        move |= piece_on_dst << 8;
    } else {
        move |= k_no_piece << 8;
    }

    move |= m_dice << 12;
    m_dice = k_no_dice;

    m_board[m_pos[piece]] = k_no_piece;
    m_board[dst] = piece;
    m_pos[piece] = dst;
    m_history[m_history_len] = move;
    m_history_len++;

    m_round_color = next_player(m_round_color);
}

void State::undo() {
    if (m_history_len <= 0) {
        fprintf(stderr, "No history to undo, aborting.\n");
        exit(1);
    }
    m_round_color = next_player(m_round_color);

    m_history_len--;
    int move = m_history[m_history_len];
    int dice = (move >> 12) & 0xf;
    int eaten_piece = (move >> 8) & 0xf;
    int piece = (move >> 4) & 0xf;
    int direction = move & 0xf;
    int src = m_pos[piece] - dir_val[m_round_color][direction];

    // #ifndef NDEBUG
    //     fprintf(stderr, "history idx: %d ", m_history_len);
    //     fprintf(stderr, "dice: %d ", dice);
    //     fprintf(stderr, "eaten_piece: %d ", eaten_piece);
    //     fprintf(stderr, "piece: %d ", piece);
    //     fprintf(stderr, "direction: %d ", direction);
    //     fprintf(stderr, "src: %d ", src);
    //     fprintf(stderr, "\n");
    // #endif

    if (eaten_piece != k_no_piece) {
        if (is_red_piece(eaten_piece)) {
            m_red_piece_num++;
        } else {
            m_blue_piece_num++;
        }
        m_board[m_pos[piece]] = eaten_piece;
        m_pos[eaten_piece] = m_pos[piece];
    } else {
        m_board[m_pos[piece]] = k_no_piece;
    }

    m_board[src] = piece;
    m_pos[piece] = src;

#ifndef NDEBUG
    assert(m_dice == k_no_dice);
#endif
    m_dice = dice;
}

int move_gen_single(move_t move_arr[], int piece, int location) {
    int count = precompute::move_gen_single_count[piece][location];
    for (int i = 0; i < count; i++) {
        move_arr[i] = precompute::move_gen_single[piece][location][i];
    }
    return count;
}

int State::move_gen_all(move_t move_arr[]) const {
    int count = 0;
    const int offset = (m_round_color == k_blue) ? 0 : k_piece_num;  // blue is 1 ~ 6
    const int* const self_pos = m_pos + offset;

    int32_t board_idx = 0;
    for (int piece = 1; piece <= k_piece_num; piece++) {
        board_idx |= static_cast<int>(self_pos[piece] != k_off_board_pos) << (piece - 1);
    }
    int moveable_count = precompute::movable_pieces_count[m_dice][board_idx];
    for (int i = 0; i < moveable_count; i++) {
        int to_move = precompute::movable_pieces[m_dice][board_idx][i];
        count += move_gen_single(move_arr + count, to_move + offset, self_pos[to_move]);
    }
    return count;
}

void State::set_dice(int dice) {
#ifndef NDEBUG
    if (m_dice != k_no_dice) {
        fprintf(stderr, "Dice is already rolled, aborting.\n");
        exit(1);
    }
#endif
    m_dice = dice;
}

void State::unset_dice() {
#ifndef NDEBUG
    if (m_dice == k_no_dice) {
        fprintf(stderr, "Dice is already reset, aborting.\n");
        exit(1);
    }
#endif
    m_dice = k_no_dice;
}

int l_inf_distance(int pos_a, int pos_b) {
    if (pos_a == -1 || pos_b == -1) {  // piece eaten, can't make it
        return 314159;
    }
    return precompute::l_inf_distance[pos_a][pos_b];
}

score_t State::evaluate() {
    // Game is over
    if (is_over()) {
        // #ifndef NDEBUG
        //         std::cerr << "Evaluating final position\n";
        // #endif
        if (winner() == m_round_color) {
            return k_max_score;
        } else {
            return k_min_score;
        }
    }

    // Minimum distance to goal
    int blue_min_dist = 100, red_min_dist = 100;

    for (int piece = 1; piece <= 6; piece++) {
        int dist = l_inf_distance(m_pos[piece], k_blue_goal);
        blue_min_dist = std::min(blue_min_dist, dist);
    }
    for (int piece = 7; piece <= 12; piece++) {
        int dist = l_inf_distance(m_pos[piece], k_red_goal);
        red_min_dist = std::min(red_min_dist, dist);
    }

    if (m_round_color == k_blue) {
        return -blue_min_dist + red_min_dist;
    } else {
        return -red_min_dist + blue_min_dist;
    }
}

void State::to_agent_move(move_t move, int agent_move[3]) const {
    int piece = (move >> 4) & 0xf;
    int direction = move & 0xf;
    int src = m_pos[piece];
    int dst = src + dir_val[m_round_color][direction];
    agent_move[0] = piece;
    agent_move[1] = src;
    agent_move[2] = dst;
}

void State::log_self() const {
    fprintf(stderr, "------ [ewn::State.log_self] -------\n");
    fprintf(stderr, "m_pos: ");
    for (int i = 1; i <= 2 * k_piece_num; i++) {
        fprintf(stderr, "%2d ", m_pos[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "-------------------\n");

    fprintf(stderr, "m_board: \n");
    for (int y = 0; y < k_board_size; y++) {
        for (int x = 0; x < k_board_size; x++) {
            int piece = m_board[y * k_board_size + x];
            if (piece <= k_piece_num) {
                fprintf(stderr, "%d ", piece);
            } else {
                fprintf(stderr, "%c ", 'A' + (piece - 7));
            }
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "-------------------\n");

    fprintf(stderr, "m_our_color: %d\n", m_our_color);
    fprintf(stderr, "m_round_color: %d\n", m_round_color);
    fprintf(stderr, "m_board_size: %d\n", m_board_size);
    fprintf(stderr, "m_dice: %d\n", m_dice);
    fprintf(stderr, "m_red_piece_num: %d\n", m_red_piece_num);
    fprintf(stderr, "m_blue_piece_num: %d\n", m_blue_piece_num);
    fprintf(stderr, "m_history_len: %d\n", m_history_len);
    fprintf(stderr, "---- [ewn::State.log_self] end -----\n");
}

bool State::check_equal(const State& rhs) const {
    if (m_our_color != rhs.m_our_color) {
        fprintf(stderr, "m_our_color not the same\n");
        goto check_equal_fail;
    }
    if (m_round_color != rhs.m_round_color) {
        fprintf(stderr, "m_round_color not the same\n");
        goto check_equal_fail;
    }
    if (m_board_size != rhs.m_board_size) {
        fprintf(stderr, "m_board_size not the same\n");
        goto check_equal_fail;
    }
    if (m_dice != rhs.m_dice) {
        fprintf(stderr, "m_dice not the same\n");
        goto check_equal_fail;
    }
    if (m_red_piece_num != rhs.m_red_piece_num) {
        fprintf(stderr, "m_red_piece_num not the same\n");
        goto check_equal_fail;
    }
    if (m_blue_piece_num != rhs.m_blue_piece_num) {
        fprintf(stderr, "m_blue_piece_num not the same\n");
        goto check_equal_fail;
    }
    if (m_history_len != rhs.m_history_len) {
        fprintf(stderr, "m_history_len not the same\n");
        goto check_equal_fail;
    }

    for (int i = 1; i <= 2 * k_piece_num; i++) {
        if (m_pos[i] != rhs.m_pos[i]) {
            fprintf(stderr, "m_pos not the same\n");
            goto check_equal_fail;
        }
    }
    for (int i = 0; i < k_board_size * k_board_size; i++) {
        if (m_board[i] != rhs.m_board[i]) {
            fprintf(stderr, "m_board not the same\n");
            goto check_equal_fail;
        }
    }

    for (int i = 0; i < m_history_len; i++) {
        if (m_history[i] != rhs.m_history[i]) {
            fprintf(stderr, "m_history not the same\n");
            goto check_equal_fail;
        }
    }

    return true;

check_equal_fail:
    fprintf(stderr, "============== check_equal failed ==============\nSee log below:\n");
    log_self();
    rhs.log_self();
    return false;
}

move_t State::find_mate(int move_arr[], int n_moves) const {
    for (int i = 0; i < n_moves; i++) {
        int move = move_arr[i];
        int piece = (move >> 4) & 0xf;
        int direction = (move) & 0xf;
        int dst = m_pos[piece] + dir_val[m_round_color][direction];

        if (m_round_color == k_red) {
            if (dst == k_red_goal) {
                return move;
            }
            if (m_blue_piece_num == 1 && is_blue_piece(m_board[dst])) {
                return move;
            }
        } else if (m_round_color == k_blue) {
            if (dst == k_blue_goal) {
                return move;
            }
            if (m_red_piece_num == 1 && is_red_piece(m_board[dst])) {
                return move;
            }
        }
    }
    return k_null_move;
}

move_t State::defend_mate(int move_arr[], int n_moves) const {
    for (int i = 0; i < n_moves; i++) {
        int move = move_arr[i];
        int piece = (move >> 4) & 0xf;
        int direction = (move) & 0xf;
        int dst = m_pos[piece] + dir_val[m_round_color][direction];

        if (m_round_color == k_red) {
            if (l_inf_distance(dst, k_red_goal) == 1 && is_blue_piece(m_board[dst])) {
                return move;
            }
        } else if (m_round_color == k_blue) {
            if (l_inf_distance(dst, k_blue_goal) == 1 && is_red_piece(m_board[dst])) {
                return move;
            }
        }
    }
    return k_null_move;
}

move_t State::shortcut() const {
    move_t move_arr[k_max_moves];
    int num_moves = move_gen_all(move_arr);

    move_t attack = find_mate(move_arr, num_moves);
    if (attack != k_null_move) {
        return attack;
    }

    move_t defend = defend_mate(move_arr, num_moves);
    if (defend != k_null_move) {
        return defend;
    }

    return k_null_move;
}

}  // namespace ewn