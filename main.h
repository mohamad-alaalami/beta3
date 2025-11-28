/*Header file that contains the function prototypes*/
#ifndef MAIN_H
#define MAIN_H
#define ROWS 6
#define COLS 7
#define TT_SIZE 524287
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

typedef unsigned long long U64;
typedef struct {
    U64 playerA;   
    U64 playerB;   
    int heights[COLS]; 
} Position;

typedef enum {
    TT_NODE_PV = 0,
    TT_NODE_CUT = 1,
    TT_NODE_ALL = 2
} TTNodeType;

typedef struct {
    uint64_t hash;
    int score;
    int depth;
    TTNodeType type;
    bool valid;
} TTEntry;

typedef struct {
    Position pos;
    int col;
    int counter;
    int depth;
    char player;
    int score;
} ThreadArgs;

extern TTEntry g_tt[TT_SIZE];
extern pthread_mutex_t tt_mutex;

void display_grid(char** grid);

int check_win(char** grid, int x, int y, char player);
int check_win_vertical(char** grid, int x, int y, char player);
int check_win_horizontal(char** grid, int x, int y, char player);
int check_win_diagonals(char** grid, int x, int y, char player);

int* update_grid(char** grid, int* capacities, int counter, char player, int bot, const char* colstring);
int* player_move(char** grid, int* capacities, char player);
int* easy_move(char** grid, int* capacities, char player);
int* medium_move(char** grid, int* capacities, char player);

int* hard_move(char** grid, int* capacities, int counter, char player, const char* colstring);
int negamax(Position* pos, int counter, int depth, char current, int alpha, int beta);
int* find_best_move(Position* pos, int counter, char player);
int get_hard_move(U64 playerA, U64 playerB, char player, int moveCount);

void game_starter(char** A, char** B,int* start_bot, int* bot);
void start_game();

Position create_bitboard(char** grid, int* capacities);
int bit_index(int row, int col);
int has_won(U64 bitboard);
void undo_move(Position *pos, int col, char current);
void play_move(Position *pos, int col, char current);

// Pascal Pons book key3 hashing helpers
uint64_t book_key3(U64 playerA, U64 playerB, char playerToMove);
void book_partialKey3(uint64_t *key, int col, U64 current_position, U64 mask);
#endif
