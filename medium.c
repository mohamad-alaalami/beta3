#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

//Template to implement in later sprint
int* medium_move(char** grid, int* capacities, char player){
    //check_possible_Vwin(grid, capacities, player);
    for(int col = 0; col < COLS - 1; col++){
        int row = ROWS - capacities[col] - 1;
        grid[row][col] = player;
        if(check_win(grid, row, col, player)){
            int* returnpos = malloc(2*sizeof(int));
            returnpos[0] = row;
            returnpos[1] = col;
            capacities[col]++;
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
    for(int col = 0; col < COLS - 1; col++){
        int row = ROWS - capacities[col] - 1;
        grid[row][col] = other;
        if(check_win(grid, row, col, other)){
            int* returnpos = malloc(2*sizeof(int));
            returnpos[0] = row;
            returnpos[1] = col;
            capacities[col]++;
            grid[row][col] = player;
            return returnpos;
        }
        else{
            grid[row][col] = '.';
        }
    }
    easy_move(grid, capacities, player);
}
/*
int* check_possible_Vwin(char** grid, int* capacities, char bot){
      int x = 100,y = 100;
      int* returnpos = malloc(sizeof(int) *2);
    for(int col = 0; col < COLS - 1; col++){
        if(capacities[col] >= 3){
            int row = ROWS - capacities[col];
            int temp_row = row;
            char player = grid[row][col];
            int counter = 0;
            while(temp_row < ROWS && grid[temp_row][col] == player){
                temp_row++;
                counter++;
            }

            if(counter == 3 && row >= 1){
                x = row - 1;
                y = col;
                if(player == bot){
                    break;
                }
            }
            
        }
    }
    if(x != 100){
        grid[x][y] = bot;
        returnpos[0] = x;
        returnpos[1] = y;
        capacities[y]++;
        printf("Bot CHOICE: %d\n\n", y + 1);
        return returnpos;
    }
    else{
        easy_move(grid, capacities, bot);
    }
    
}*/

