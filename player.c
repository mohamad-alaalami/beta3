#include <stdio.h>
#include <stdlib.h>
#include "main.h"

/*function that takes as input the column choice of the user if it is a valid choice(in range and an integer) it checks the capacity of the column and places it in the lowest available slot*/
int* player_move(char** grid, int* capacities, char player){
    int choice;

    int* returnpos = (int*)malloc(2*sizeof(int));
    while(1){
        printf("Player %c, please choose a column: ", player);
        if(scanf("%d", &choice) != 1){
            fflush(stdout);
            printf("enter an integer.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        if(choice > 0 && choice <= COLS){
            if(capacities[choice - 1] < ROWS){
                int row = ROWS - capacities[choice - 1] - 1;
                grid[row][choice - 1] = player;
                capacities[choice - 1]++;


                //returnpos[0]= ROWS-capacities[choice]-1;
                returnpos[0]=row;
                returnpos[1]= choice - 1;
                return returnpos;
        }

            else{
                printf("column is full.\n");
            }
        }

        else{
            printf("invalid column.\n");
        }
    }
}