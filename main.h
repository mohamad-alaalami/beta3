/*Header file that contains the function prototypes*/
#ifndef MAIN_H
#define MAIN_H
#define ROWS 6
#define COLS 7

typedef unsigned long long U64;

typedef struct {
    U64 playerA;   
    U64 playerB;   
    int heights[COLS]; 
} Position;


void display_grid(char** grid);

int check_win(char** grid, int x, int y, char player);
int check_win_vertical(char** grid, int x, int y, char player);
int check_win_horizontal(char** grid, int x, int y, char player);
int check_win_diagonals(char** grid, int x, int y, char player);

int* update_grid(char** grid, int* capacities, int counter, char player, int bot);
int* player_move(char** grid, int* capacities, char player);
int* easy_move(char** grid, int* capacities, char player);
int* medium_move(char** grid, int* capacities, char player);

int* hard_move(char** grid, int* capacities, int counter, char player);
int negamax(Position* pos, int counter, char current, int alpha, int beta);
int* find_best_move(Position* pos, int counter, char player);

void game_starter(char** A, char** B,int* random, int* bot);
void start_game();

Position create_bitboard(char** grid, int* capacities);
int bit_index(int row, int col);
int has_won(U64 bitboard);
void undo_move(Position *pos, int col, char current);
void play_move(Position *pos, int col, char current);
#endif