#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

//Template to implement in later sprint
int* hard_move(char** grid, int* capacities, int counter, char player){
    if(counter == 0){
        int choice = COLS / 2;
        printf("Bot choice: %d\n\n", choice + 1);
        int row = ROWS - capacities[choice] - 1;
        grid[row][choice] = player;
        capacities[choice]++;

        int* returnpos = malloc(2*sizeof(int));
        returnpos[0]=row;
        returnpos[1]= choice;
        return returnpos;
    }
    return find_best_move(grid, capacities, counter, player);
}


int minimax(char** tempGrid, int* capacitites, int isMaximizing, int counter, int player, int opponent, int r, int c, char bot){

    if(check_win(tempGrid, r, c, opponent)){
        if(bot == opponent){
            return 1;
        }
        else{
            return -1;
        }
    }

    else if (counter >= 42){
        return 0;
    }
    
    if (isMaximizing){
        int bestScore = -1000;
        for(int col = 0; col < COLS; col++){
            int row = ROWS - capacitites[col] - 1;
            if(row < 0){
                continue;
            }

            tempGrid[row][col] = player;
            capacitites[col]++;
            int score = minimax(tempGrid, capacitites, 0, counter + 1, opponent, player, row, col, bot);
            tempGrid[row][col] = '.';
            capacitites[col]--;
            if(score > bestScore){
                bestScore = score;
            }
        }
        return bestScore;
    }
    else{
        int bestScore = 1000;
        for(int col = 0; col < COLS; col++){
            int row = ROWS - capacitites[col] - 1;
            if(row < 0){
                continue;
            }

            tempGrid[row][col] = opponent;
            capacitites[col]++;
            int score = minimax(tempGrid, capacitites, 1, counter + 1, opponent, player, row, col, bot);
            tempGrid[row][col] = '.';
            capacitites[col]--;
            if(score < bestScore){
                bestScore = score;
            }
        }
        return bestScore;
    }
}


int* find_best_move(char** grid, int* capacities, int counter, char player){
    char opponent = (player == 'A') ? 'A' : 'B';
    int bestScore = -1000;
    int* bestMove = (int*)malloc(2 * sizeof(int));
    bestMove[0] = -1;
    bestMove[1] = -1;

    for(int col = 0; col < COLS; col++){
        int row = ROWS - capacities[col] - 1;
        if(row < 0){
            continue;
        }

        grid[row][col] = player;
        capacities[col]++;
        int score = minimax(grid, capacities, 0, counter + 1, player, opponent, row, col, player);
        grid[row][col] = '.';
        capacities[col]--;

        if(score > bestScore){
            bestScore = score;
            bestMove[0] = row;
            bestMove[1] = col;
        }
    }
    printf("Bot choice: %d\n\n", col + 1);
    return bestMove;
}