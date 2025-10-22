#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

//Template to implement in later sprint
int* medium_move(char** grid, int* capacities, char player){
    int choice;
    
    do{
        choice = (rand() % 8) + 1;

    }while(capacities[choice - 1] < ROWS);

    int row = ROWS - capacities[choice - 1] - 1;
    grid[row][choice - 1] = player;
    capacities[choice - 1]++;

    int* returnpos = malloc(2*sizeof(int));
    returnpos[0]=row;
    returnpos[1]= choice - 1;
    return returnpos;
}