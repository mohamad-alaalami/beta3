#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

/*Easy Bot:
    Makes a random column choice
    Checks if a valid choice
    Plays it
The Logic behind implementing this bot is that it is completely random and has no basis
in choosing a column.
*/
int* easy_move(char** grid, int* capacities, char player){
    int choice;
    srand(time(NULL));

    do{
        choice = (rand() % 7) + 1;
    }while(capacities[choice - 1] > ROWS);

    printf("Bot choice: %d\n\n", choice);
    int row = ROWS - capacities[choice - 1] - 1;
    grid[row][choice - 1] = player;
    capacities[choice - 1]++;

    int* returnpos = malloc(2*sizeof(int));
    returnpos[0]=row;
    returnpos[1]= choice - 1;
    return returnpos;
}