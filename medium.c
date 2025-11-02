#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

/*Medium Bot:
The Logic behind implementing this bot is that it simulates all the possible
and valid moves it can make during its turn to win firstly as it favors winning;
else it simulates again to see if it can block the player from winning;
if it can not win or block then it makes a random move.
*/
int* medium_move(char** grid, int* capacities, char player){
    //check_possible_win(grid, capacities, player);
    for(int col = 0; col < COLS; col++){
        int row = ROWS - capacities[col] - 1;
        if(row < 0){
            continue;
        }

        grid[row][col] = player;
        if(check_win(grid, row, col, player)){
            int* returnpos = malloc(2*sizeof(int));
            returnpos[0] = row;
            returnpos[1] = col;
            capacities[col]++;
            printf("Bot choice: %d\n\n", col + 1);
            return returnpos;
        }
        else{
            grid[row][col] = '.';
        }
    }
    char other;
    if(player == 'A'){
        other = 'B';
    }
    else{
        other = 'A';
    }

    for(int col = 0; col < COLS; col++){
        int row = ROWS - capacities[col] - 1;
        if(row < 0){
            continue;
        }

        grid[row][col] = other;
        if(check_win(grid, row, col, other)){
            int* returnpos = malloc(2*sizeof(int));
            returnpos[0] = row;
            returnpos[1] = col;
            capacities[col]++;
            grid[row][col] = player;
            printf("Bot choice: %d\n\n", col + 1);
            return returnpos;
        }
        else{
            grid[row][col] = '.';
        }
    }
    easy_move(grid, capacities, player);
}

