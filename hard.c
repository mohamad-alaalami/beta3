#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "main.h"
#include "opening_book.h"

extern OpeningBook BOOK;

static const int move_order[7] = {3, 2, 4, 1, 5, 0, 6};
static const int MAX_SEARCH_DEPTH = 14; // Upper bound for iterative deepening
static const int WIN_SCORE = 100000;
static const int INF_SCORE = 200000;
static const double MOVE_TIME_LIMIT = 15.0;

static struct timespec g_move_start;
static volatile bool g_time_over = false;

// Precomputed center column bits (col = 3) to keep initializer constant
static const U64 CENTER_MASK =
    (1ULL << 21) | (1ULL << 22) | (1ULL << 23) |
    (1ULL << 24) | (1ULL << 25) | (1ULL << 26);

static U64 g_win_masks[69];
static bool g_win_masks_ready = false;

pthread_mutex_t tt_mutex = PTHREAD_MUTEX_INITIALIZER;
TTEntry g_tt[TT_SIZE];

static inline int bit_popcount(U64 v) {
    return __builtin_popcountll(v);
}

static inline double elapsed_seconds(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double sec = (double)(now.tv_sec - g_move_start.tv_sec);
    double nsec = (double)(now.tv_nsec - g_move_start.tv_nsec) / 1e9;
    return sec + nsec;
}

static inline void start_timer(void) {
    clock_gettime(CLOCK_MONOTONIC, &g_move_start);
    g_time_over = false;
}

static inline bool time_is_up(void) {
    if (g_time_over) {
        return true;
    }
    if (elapsed_seconds() >= MOVE_TIME_LIMIT) {
        g_time_over = true;
        return true;
    }
    return false;
}

static inline void init_win_masks(void) {
    if (g_win_masks_ready) return;
    int idx = 0;
    for (int r = 0; r < ROWS; r++) {              // horizontal
        for (int c = 0; c <= COLS - 4; c++) {
            U64 mask = 0;
            for (int k = 0; k < 4; k++) {
                mask |= 1ULL << bit_index(r, c + k);
            }
            g_win_masks[idx++] = mask;
        }
    }
    for (int c = 0; c < COLS; c++) {              // vertical
        for (int r = 0; r <= ROWS - 4; r++) {
            U64 mask = 0;
            for (int k = 0; k < 4; k++) {
                mask |= 1ULL << bit_index(r + k, c);
            }
            g_win_masks[idx++] = mask;
        }
    }
    for (int r = 0; r <= ROWS - 4; r++) {         // diag down-right
        for (int c = 0; c <= COLS - 4; c++) {
            U64 mask = 0;
            for (int k = 0; k < 4; k++) {
                mask |= 1ULL << bit_index(r + k, c + k);
            }
            g_win_masks[idx++] = mask;
        }
    }
    for (int r = 0; r <= ROWS - 4; r++) {         // diag down-left
        for (int c = 3; c < COLS; c++) {
            U64 mask = 0;
            for (int k = 0; k < 4; k++) {
                mask |= 1ULL << bit_index(r + k, c - k);
            }
            g_win_masks[idx++] = mask;
        }
    }
    g_win_masks_ready = true;
}

static inline U64 playable_mask(const Position* pos) {
    U64 mask = 0;
    for (int c = 0; c < COLS; c++) {
        int h = pos->heights[c];
        if (h < ROWS) {
            mask |= 1ULL << bit_index(h, c);
        }
    }
    return mask;
}

static inline U64 winning_spots(U64 me, U64 opp) {
    init_win_masks();
    U64 wins = 0;
    U64 all = me | opp;
    for (int i = 0; i < 69; i++) {
        U64 mask = g_win_masks[i];
        if (mask & opp) continue;
        U64 empty = mask & ~all;
        if (empty && ((empty & (empty - 1ULL)) == 0ULL)) {
            wins |= empty;
        }
    }
    return wins;
}

static inline int count_threats(U64 me, U64 opp) {
    init_win_masks();
    int threes = 0;
    int twos = 0;
    for (int i = 0; i < 69; i++) {
        U64 mask = g_win_masks[i];
        if (mask & opp) continue;
        int cnt = bit_popcount(me & mask);
        if (cnt == 3) {
            threes++;
        } else if (cnt == 2) {
            twos++;
        }
    }
    return threes * 6 + twos * 2;
}

static inline int count_double_threats(const Position* pos, U64 me, U64 opp, U64 playable) {
    int count = 0;
    for (int c = 0; c < COLS; c++) {
        int h = pos->heights[c];
        if (h >= ROWS) continue;
        U64 move = 1ULL << bit_index(h, c);
        U64 new_me = me | move;
        U64 new_opp = opp;
        U64 new_playable = playable & ~move;
        if (h + 1 < ROWS) {
            new_playable |= 1ULL << bit_index(h + 1, c);
        }
        U64 threats = winning_spots(new_me, new_opp) & new_playable;
        if (bit_popcount(threats) >= 2) {
            count++;
        }
    }
    return count;
}

static inline int evaluate_position(const Position* pos, char current, int counter) {
    U64 me = (current == 'A') ? pos->playerA : pos->playerB;
    U64 opp = (current == 'A') ? pos->playerB : pos->playerA;

    if (has_won(me)) return WIN_SCORE - counter;
    if (has_won(opp)) return -WIN_SCORE + counter;

    U64 playable = playable_mask(pos);
    U64 my_wins = winning_spots(me, opp);
    U64 opp_wins = winning_spots(opp, me);

    int score = 0;
    score += bit_popcount(my_wins & playable) * 80;
    score -= bit_popcount(opp_wins & playable) * 95;

    int center = bit_popcount(me & CENTER_MASK) - bit_popcount(opp & CENTER_MASK);
    score += center * 10;

    int parity = ((ROWS * COLS - counter) & 1) ? 6 : -6;
    score += parity;

    score += count_threats(me, opp);
    score -= count_threats(opp, me);

    int my_double = count_double_threats(pos, me, opp, playable);
    int opp_double = count_double_threats(pos, opp, me, playable);
    score += my_double * 120;
    score -= opp_double * 140;

    return score;
}

static uint64_t tt_hash(const Position* pos, char current) {
    uint64_t hash = pos->playerA * 11400714819323198485ull;
    hash ^= pos->playerB * 14029467366897019727ull;
    hash ^= ((uint64_t)current * 1609587929392839161ull);
    return hash;
}

static unsigned tt_index(uint64_t hash) {
    return (unsigned)(hash % TT_SIZE);
}

static bool tt_lookup(const Position* pos, char current, int depth, int alpha, int beta, int* score) {
    uint64_t hash = tt_hash(pos, current);
    unsigned idx = tt_index(hash);
    TTEntry entry = g_tt[idx];

    if (!entry.valid || entry.hash != hash || entry.depth < depth) {
        return false;
    }

    if (entry.type == TT_NODE_PV) {
        *score = entry.score;
        return true;
    }

    if (entry.type == TT_NODE_CUT && entry.score >= beta) {
        *score = entry.score;
        return true;
    }

    if (entry.type == TT_NODE_ALL && entry.score <= alpha) {
        *score = entry.score;
        return true;
    }

    return false;
}

static void tt_store(const Position* pos, char current, int depth, int score, TTNodeType type) {
    uint64_t hash = tt_hash(pos, current);
    unsigned idx = tt_index(hash);

    pthread_mutex_lock(&tt_mutex);
    TTEntry* entry = &g_tt[idx];
    if (!entry->valid || entry->hash != hash || depth >= entry->depth) {
        entry->hash = hash;
        entry->score = score;
        entry->depth = depth;
        entry->type = type;
        entry->valid = true;
    }
    pthread_mutex_unlock(&tt_mutex);
}



static inline int heuristic_move_score(const Position* pos, char current, int col, int counter) {
    int row = pos->heights[col];
    if (row >= ROWS) return -INF_SCORE;

    U64 me = (current == 'A') ? pos->playerA : pos->playerB;
    U64 opp = (current == 'A') ? pos->playerB : pos->playerA;
    U64 playable = playable_mask(pos);
    U64 move = 1ULL << bit_index(row, col);

    if (has_won(me | move)) {
        return WIN_SCORE;
    }

    U64 opp_immediate = winning_spots(opp, me) & playable;
    if (opp_immediate & move) {
        return WIN_SCORE / 2;
    }

    U64 new_me = me | move;
    U64 new_opp = opp;
    U64 new_playable = (playable & ~move);
    if (row + 1 < ROWS) {
        new_playable |= 1ULL << bit_index(row + 1, col);
    }

    int double_threats = bit_popcount(winning_spots(new_me, new_opp) & new_playable);
    int center_bias = (col == 3) ? 6 : (col == 2 || col == 4) ? 4 : 2;
    int base_eval = center_bias * 2 + double_threats * 40;

    return base_eval + evaluate_position(pos, current, counter);
}

static int column_height_from_mask(U64 mask, int col) {
    for (int row = 0; row < ROWS; row++) {
        if ((mask & (1ULL << bit_index(row, col))) == 0) {
            return row;
        }
    }
    return ROWS;
}

int get_hard_move(U64 playerA, U64 playerB, char player, int moveCount) {
    int book_move = opening_book_best_move(playerA, playerB, player, moveCount, &BOOK);
    if (book_move >= 0) {
        printf("[BOOK HIT] move = %d\n", book_move);
        return book_move;
    }

    Position pos;
    pos.playerA = playerA;
    pos.playerB = playerB;
    U64 mask = playerA | playerB;
    for (int c = 0; c < COLS; c++) {
        pos.heights[c] = column_height_from_mask(mask, c);
    }

    int* best = find_best_move(&pos, moveCount, player);
    if (!best || best[1] == -1) {
        if (best) free(best);
        return -1;
    }
    int col = best[1];
    free(best);
    return col;
}

int* hard_move(char** grid, int* capacities, int counter, char player){
    int* move = malloc(2 * sizeof(int));
    if (!move) {
        return NULL;
    }

    Position pos = create_bitboard(grid, capacities);
    int best_col = get_hard_move(pos.playerA, pos.playerB, player, counter);
    int row;
    if (best_col >= 0 && capacities[best_col] < ROWS) {
        row = ROWS - capacities[best_col] - 1;
    } else {
        int* best = find_best_move(&pos, counter, player);
        if (!best || best[1] == -1) {
            free(best);
            free(move);
            return easy_move(grid, capacities, player);
        }
        row = best[0];
        best_col = best[1];
        free(best);
    }

    grid[row][best_col] = player;
    capacities[best_col]++;

    move[0] = row;
    move[1] = best_col;
    printf("Bot choice: %d\n\n", best_col + 1);
    return move;
}




void* thread_worker(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;

    // Make local copy of position so threads don’t fight
    Position local = args->pos;

    // Apply the move for this thread’s column
    play_move(&local, args->col, args->player);

    char opponent = (args->player == 'A') ? 'B' : 'A';
    int remainingDepth = args->depth - 1;
    if (remainingDepth < 0) {
        remainingDepth = 0;
    }

    // Do negamax
    args->score = -negamax(&local, args->counter + 1, remainingDepth, opponent, -INF_SCORE, INF_SCORE);

    return NULL;
}


int negamax(Position* pos, int counter, int depth, char current, int alpha, int beta){
    if (time_is_up()) {
        return evaluate_position(pos, current, counter);
    }

    int cached;
    if(tt_lookup(pos, current, depth, alpha, beta, &cached)){
        return cached;
    }

    char opponent = (current == 'A') ? 'B' : 'A';

    if(has_won(current == 'A' ? pos->playerB : pos->playerA)){
        int loss = -WIN_SCORE + counter;
        tt_store(pos, current, depth, loss, TT_NODE_PV);
        return loss;
    }

    if(counter >= ROWS * COLS){
        tt_store(pos, current, depth, 0, TT_NODE_PV);
        return 0;
    }

    if (depth <= 0) {
        int eval = evaluate_position(pos, current, counter);
        tt_store(pos, current, depth, eval, TT_NODE_PV);
        return eval;
    }

    int originalAlpha = alpha;
    int bestScore = -INF_SCORE;
    bool moveFound = false;

    int cols[COLS];
    int scores[COLS];
    int moveCount = 0;

    for (int k = 0; k < COLS; k++) {
        int col = move_order[k];
        int row = pos->heights[col];
        if (row >= ROWS) continue;
        cols[moveCount] = col;
        scores[moveCount] = heuristic_move_score(pos, current, col, counter);
        moveCount++;
    }

    for (int i = 1; i < moveCount; i++) { // insertion sort desc by heuristic
        int c = cols[i];
        int s = scores[i];
        int j = i - 1;
        while (j >= 0 && scores[j] < s) {
            cols[j + 1] = cols[j];
            scores[j + 1] = scores[j];
            j--;
        }
        cols[j + 1] = c;
        scores[j + 1] = s;
    }

    for (int idx = 0; idx < moveCount; idx++) {
        int col = cols[idx];

        moveFound = true;
        play_move(pos, col, current);

        int score = -negamax(pos, counter + 1, depth - 1, opponent, -beta, -alpha);

        undo_move(pos, col, current);

        if(score > bestScore){
            bestScore = score;
        }
        if(bestScore > alpha){
            alpha = bestScore;
        }
        if(alpha >= beta){
            break;
        }
        if (time_is_up()) {
            break;
        }
    }

    if(!moveFound){
        tt_store(pos, current, depth, 0, TT_NODE_PV);
        return 0;
    }

    TTNodeType nodeType;
    if(bestScore <= originalAlpha){
        nodeType = TT_NODE_ALL;
    } else if(bestScore >= beta){
        nodeType = TT_NODE_CUT;
    } else {
        nodeType = TT_NODE_PV;
    }

    tt_store(pos, current, depth, bestScore, nodeType);
    return bestScore;
}


int* find_best_move(Position* pos, int counter, char player){
    int remainingMoves = ROWS * COLS - counter;
    if (remainingMoves <= 0) {
        int* fallback = malloc(sizeof(int) * 2);
        fallback[0] = -1;
        fallback[1] = -1;
        return fallback;
    }

    if (counter <= 16) {
        uint64_t key = book_key3(pos->playerA, pos->playerB, player);
        int bookMove = opening_book_lookup_key(key, &BOOK);
        if (bookMove >= 0 && bookMove < COLS && pos->heights[bookMove] < ROWS) {
            int row = ROWS - pos->heights[bookMove] - 1;
            int* result = malloc(sizeof(int) * 2);
            result[0] = row;
            result[1] = bookMove;
            printf("[BOOK] col %d\n", bookMove + 1);
            return result;
        }
    }

    start_timer();

    int maxDepth;
    if (counter > 13) {
        maxDepth = remainingMoves;
    } else {
        maxDepth = remainingMoves < MAX_SEARCH_DEPTH ? remainingMoves : MAX_SEARCH_DEPTH;
    }
    if (maxDepth < 1) {
        maxDepth = 1;
    }

    int root_moves[COLS];
    int root_count = 0;
    for (int k = 0; k < COLS; k++) {
        int col = move_order[k];
        if (pos->heights[col] >= ROWS) {
            continue;
        }
        root_moves[root_count++] = col;
    }

    if (root_count == 0) {
        int* fallback = malloc(sizeof(int) * 2);
        fallback[0] = -1;
        fallback[1] = -1;
        return fallback;
    }

    int root_scores[COLS];
    for (int i = 0; i < root_count; i++) {
        root_scores[i] = heuristic_move_score(pos, player, root_moves[i], counter);
    }
    for (int i = 1; i < root_count; i++) { // sort desc
        int col = root_moves[i];
        int sc = root_scores[i];
        int j = i - 1;
        while (j >= 0 && root_scores[j] < sc) {
            root_moves[j + 1] = root_moves[j];
            root_scores[j + 1] = root_scores[j];
            j--;
        }
        root_moves[j + 1] = col;
        root_scores[j + 1] = sc;
    }

    int bestCol = root_moves[0];
    int bestScore = -INF_SCORE;
    int searchedDepth = 0;
    int pv_move = -1;

    for (int depth = 1; depth <= maxDepth; depth++) {
        if (time_is_up()) {
            break;
        }
        if (pv_move != -1) {
            int pv_index = -1;
            for (int i = 0; i < root_count; i++) {
                if (root_moves[i] == pv_move) {
                    pv_index = i;
                    break;
                }
            }
            if (pv_index > 0) {
                int pv_col = root_moves[pv_index];
                for (int j = pv_index; j > 0; j--) {
                    root_moves[j] = root_moves[j - 1];
                }
                root_moves[0] = pv_col;
            }
        }

        pthread_t threads[COLS];
        ThreadArgs tasks[COLS];
        bool thread_started[COLS] = {false};

        for (int i = 0; i < root_count; i++) {
            int col = root_moves[i];
            ThreadArgs* task = &tasks[i];
            task->pos = *pos;
            task->col = col;
            task->counter = counter;
            task->depth = depth;
            task->player = player;
            task->score = -INF_SCORE;

            if (pthread_create(&threads[i], NULL, thread_worker, task) == 0) {
                thread_started[i] = true;
            } else {
                thread_started[i] = false;
                thread_worker(task);
            }
        }

        for (int i = 0; i < root_count; i++) {
            if (thread_started[i]) {
                pthread_join(threads[i], NULL);
            }
        }

        int depthBestScore = -INF_SCORE;
        int depthBestCol = -1;

        for (int i = 0; i < root_count; i++) {
            int score = tasks[i].score;
            if (score > depthBestScore) {
                depthBestScore = score;
                depthBestCol = tasks[i].col;
            }
        }

        if (depthBestCol != -1) {
            bestCol = depthBestCol;
            bestScore = depthBestScore;
            searchedDepth = depth;
            pv_move = depthBestCol;
        }

        if (bestScore >= WIN_SCORE / 2 || time_is_up()) {
            break;
        }
    }

    if (bestCol == -1) {
        int* fallback = malloc(sizeof(int) * 2);
        fallback[0] = -1;
        fallback[1] = -1;
        return fallback;
    }

    int row = ROWS - pos->heights[bestCol] - 1;
    int* result = malloc(sizeof(int) * 2);
    result[0] = row;
    result[1] = bestCol;

    printf("Bot choice: %d (depth %d)\n\n", bestCol + 1, searchedDepth);
    return result;
}






void play_move(Position *pos, int col, char current) {
    int row = pos->heights[col];
    // if row >= ROWS, column is full – handle that outside
    int idx = bit_index(row, col);
    U64 mask = 1ULL << idx;

    if (current == 'A')
        pos->playerA |= mask;
    else
        pos->playerB |= mask;

    pos->heights[col]++;
}




void undo_move(Position *pos, int col, char current) {
    pos->heights[col]--;
    int row = pos->heights[col];
    int idx = bit_index(row, col);
    U64 mask = 1ULL << idx;

    if (current == 'A')
        pos->playerA &= ~mask;
    else
        pos->playerB &= ~mask;
}



int has_won(U64 bitboard) {
    U64 y;

    // vertical (shift by 1)
    y = bitboard & (bitboard >> 1);
    if (y & (y >> 2)) return 1;

    // diagonal /
    y = bitboard & (bitboard >> 6);
    if (y & (y >> 2 * 6)) return 1;

    // horizontal
    y = bitboard & (bitboard >> 7);
    if (y & (y >> 2 * 7)) return 1;

    // diagonal 
    y = bitboard & (bitboard >> 8);
    if (y & (y >> 2 * 8)) return 1;

    return 0;
}

int bit_index(int row, int col) {
    return row + col * 7; 
}


Position create_bitboard(char** grid, int* capacities) {
    Position pos;
    pos.playerA = 0ULL;
    pos.playerB = 0ULL;

    for (int col = 0; col < COLS; col++) {
        pos.heights[col] = capacities[col];
        for (int depth = 0; depth < capacities[col]; depth++) {
            int grid_row = ROWS - 1 - depth;
            char cell = grid[grid_row][col];
            int idx = bit_index(depth, col);
            if (cell == 'A') {
                pos.playerA |= (1ULL << idx);
            } else if (cell == 'B') {
                pos.playerB |= (1ULL << idx);
            }
        }
    }

    return pos;
}
