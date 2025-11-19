#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "main.h"

static const int move_order[7] = {3, 2, 4, 1, 5, 0, 6};

#define TT_SIZE 524287

typedef struct {
    U64 playerA;
    U64 playerB;
    char current;
    int score;
    bool valid;
} TTEntry;

static TTEntry transposition_table[TT_SIZE];

static void tt_reset(void) {
    memset(transposition_table, 0, sizeof(transposition_table));
}

static unsigned tt_index(const Position* pos, char current) {
    uint64_t hash = pos->playerA * 11400714819323198485ull;
    hash ^= pos->playerB * 14029467366897019727ull;
    hash ^= (uint64_t)current;
    return (unsigned)(hash % TT_SIZE);
}

static bool tt_lookup(const Position* pos, char current, int* score) {
    unsigned idx = tt_index(pos, current);
    TTEntry* entry = &transposition_table[idx];
    if(entry->valid && entry->playerA == pos->playerA && entry->playerB == pos->playerB && entry->current == current) {
        *score = entry->score;
        return true;
    }
    return false;
}

static void tt_store(const Position* pos, char current, int score) {
    unsigned idx = tt_index(pos, current);
    TTEntry* entry = &transposition_table[idx];
    entry->valid = true;
    entry->playerA = pos->playerA;
    entry->playerB = pos->playerB;
    entry->current = current;
    entry->score = score;
}

int* hard_move(char** grid, int* capacities, int counter, char player){
    int* returnpos = malloc(2*sizeof(int));
    if(counter == 0){
        int choice = COLS / 2;
        printf("Bot choice: %d\n\n", choice + 1);
        int row = ROWS - capacities[choice] - 1;
        grid[row][choice] = player;
        capacities[choice]++;

        returnpos[0]=row;
        returnpos[1]= choice;
        return returnpos;
    }
    if(counter <= 5){return player_move(grid, capacities, player);}

    tt_reset();
    Position pos = create_bitboard(grid, capacities);
    returnpos = find_best_move(&pos, counter, player);
    grid[returnpos[0]][returnpos[1]] = player;
    capacities[returnpos[1]]++; 

    return returnpos;
}


int negamax(Position* pos, int counter, char current, int alpha, int beta){
    int cached;
    if(tt_lookup(pos, current, &cached)){
        return cached;
    }

    char opponent = (current == 'A') ? 'B' : 'A';

    if(has_won(current == 'A' ? pos->playerB : pos->playerA)){
        tt_store(pos, current, -1);
        return -1;
    }

    if(counter >= ROWS * COLS){
        tt_store(pos, current, 0);
        return 0;
    }

    int bestScore = -1000;

    for (int k = 0; k < COLS; k++) {
        int col = move_order[k];
        int row = ROWS - pos->heights[col] - 1;
        if(row < 0){
            continue;
        }

        play_move(pos, col, current);

        int score = -negamax(pos, counter + 1, opponent, -beta, -alpha);

    
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

    if(bestScore == -1000){
        tt_store(pos, current, 0);
        return 0;
    }

    tt_store(pos, current, bestScore);
    return bestScore;
}


int* find_best_move(Position* pos, int counter, char player){
    char opponent = (player == 'A') ? 'B' : 'A';
    int bestScore = -1000;
    int* returnpos = (int*)malloc(2 * sizeof(int));
    returnpos[0] = -1;
    returnpos[1] = -1;
    int col;
    for (int k = 0; k < COLS; k++) {
        int col = move_order[k];
        int row = ROWS - pos->heights[col] - 1;
        if(row < 0){
            continue;
        }

        play_move(pos, col, player);

        int score = -negamax(pos, counter + 1, opponent, -1000, 1000);
        
        undo_move(pos, col, player);

        if(score > bestScore){
            bestScore = score;
            returnpos[0] = row;
            returnpos[1] = col;
        }
    }

    if(returnpos[1] != -1){
        printf("Bot choice: %d\n\n", returnpos[1] + 1);
    }
    return returnpos;
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