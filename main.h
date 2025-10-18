/*Header file that contains the function prototypes*/
#ifndef MAIN_H
#define MAIN_H
#define ROWS 6
#define COLS 7

void display_grid(char** grid);

int check_win(char** grid, int x, int y, char player);
int check_win_vertical(char** grid, int x, int y, char player);
int check_win_horizontal(char** grid, int x, int y, char player);
int check_win_diagonals(char** grid, int x, int y, char player);

int* update_grid(char** grid, int* capacities, char player, int bot);
int* player_move(char** grid, int* capacities, char player);
int* easy_move(char** grid, int* capacities, char player);
int* medium_move(char** grid, int* capacities, char player);
int* hard_move(char** grid, int* capacities, char player);

void start_game();
#endif