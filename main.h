#ifndef define MAIN_H
#define MAIN_H
#define ROWS 6
#define COLS 7

void display_grid(char** grid);
void update_grid(char** grid);
int is_valid(char** grid, int x, int y);
int check_win(char** grid, int x, int y);
int check_win_vertical(char** grid, int x, int y);
int check_win_horizontal(char** grid, int x, int y);
int check_win_diagonals(char** grid, int x, int y);
void start_game();

#endif