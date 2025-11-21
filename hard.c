#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "main.h"

static const int move_order[7] = {3, 2, 4, 1, 5, 0, 6};
static const int MAX_SEARCH_DEPTH = 14; // Upper bound for iterative deepening

pthread_mutex_t tt_mutex = PTHREAD_MUTEX_INITIALIZER;
TTEntry g_tt[TT_SIZE];

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




int* hard_move(char** grid, int* capacities, int counter, char player){
    int* move = malloc(2 * sizeof(int));
    if (!move) {
        return NULL;
    }

    if(counter == 0){
        int choice = COLS / 2;
        int row = ROWS - capacities[choice] - 1;
        grid[row][choice] = player;
        capacities[choice]++;

        move[0] = row;
        move[1] = choice;
        printf("Bot choice: %d (depth 1)\n\n", choice + 1);
        return move;
    }

    Position pos = create_bitboard(grid, capacities);
    int* best = find_best_move(&pos, counter, player);
    if (!best || best[1] == -1) {
        free(best);
        free(move);
        return easy_move(grid, capacities, player);
    }

    grid[best[0]][best[1]] = player;
    capacities[best[1]]++;

    free(move);
    return best;
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
    args->score = -negamax(&local, args->counter + 1, remainingDepth, opponent, -1000, 1000);

    return NULL;
}


int negamax(Position* pos, int counter, int depth, char current, int alpha, int beta){
    int cached;
    if(tt_lookup(pos, current, depth, alpha, beta, &cached)){
        return cached;
    }

    char opponent = (current == 'A') ? 'B' : 'A';

    if(has_won(current == 'A' ? pos->playerB : pos->playerA)){
        tt_store(pos, current, depth, -1, TT_NODE_PV);
        return -1;
    }

    if(counter >= ROWS * COLS){
        tt_store(pos, current, depth, 0, TT_NODE_PV);
        return 0;
    }

    if (depth <= 0) {
        tt_store(pos, current, depth, 0, TT_NODE_PV);
        return 0;
    }

    int originalAlpha = alpha;
    int bestScore = -1000;
    bool moveFound = false;

    for (int k = 0; k < COLS; k++) {
        int col = move_order[k];
        int row = pos->heights[col];
        if(row >= ROWS){
            continue;
        }

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

    int bestCol = root_moves[0];
    int bestScore = -1000;
    int searchedDepth = 0;
    int pv_move = -1;

    for (int depth = 1; depth <= maxDepth; depth++) {
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
            task->score = -1000;

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

        int depthBestScore = -1000;
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

        if (bestScore == 1) {
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
