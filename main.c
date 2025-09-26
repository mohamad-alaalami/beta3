#include <stdio.h>
#include <stdlib.h>
#include "main.h"

/*Function that displays the 6 by 7 board of the game*/
void display_grid(char **grid){
	for(int i = 0;i < ROWS;i++){
		for(int j = 0;j < COLS;j++){
			printf("%c  ", grid[i][j]);
		}
		printf("\n");
	}
	printf("1  2  3  4  5  6  7\n");
}

/*Function that takes the position of the last placed char by the player and checks if that vertical contains 4 in a row*/
int check_win_vertical(char** grid, int x, int y, char player){
    if(x >= 3){
        return 0;
    }

    int counter = 0;
    for(int i = x; i <= 5; i++){
        if(grid[i][y] == player){
            counter++;
        }
        else{
            return 0;
        }
        if(counter == 4){
            return 1;
        }
    }

    return 0;
}

/*Function that takes the position of the last placed char by the player and checks if that horizontal contains 4 in a row*/
int check_win_horizontal(char** grid, int x, int y, char player){
    int counter = 0;
    for(int i = 0; i < COLS; i++){
        if(grid[x][i] == player){
            counter++;
            if(counter == 4){
                return 1;
            }
        }
        else{
            counter = 0;
        }
    }

    return 0;
}

/*Function that takes the position of the last placed char by the player and checks if either diagonal contains 4 in a row*/
int check_win_diagonals(char **grid, int x, int y, char player){
    //to not modify the position to be used later
	int temp_x = x;
	int temp_y = y;
	int counter;

    /*going to the lower leftmost availabe slot on the diagonal the player placed the
    char in*/ 
	while(temp_x > 0 && temp_x < 5 && temp_y > 0 && temp_y < 6){
		temp_x++;
		temp_y--;
	}

    /*setting counter to 0 and looping over the diagonal(going up then right) and incrementing counter if we view a char same as the one the player placed*/
	counter = 0;
	while(temp_x >= 0 && temp_x <= 5 && temp_y >= 0 && temp_y <= 6 ){
		
		if(grid[temp_x][temp_y] == player){
			counter++;
		}
		else{
			counter = 0;
		}
		if(counter == 4){
			return 1;
			}
		temp_x--;
		temp_y++;
	}

    //resetting the temp variables to the initial position to use again 
	temp_x = x;
	temp_y = y;

    /*going to the upper rightmost availabe slot on the diagonal the player placed the
    char in*/ 
	while(temp_x > 0 && temp_x < 5 && temp_y > 0 && temp_y < 6 ){
		temp_x++;
		temp_y++;
	}
    
    /*setting counter to 0 and looping over the diagonal(going down then left) and incrementing counter if we view a char same as the one the player placed*/
	counter = 0;
	while(temp_x >= 0 && temp_x <= 5 && temp_y >= 0 && temp_y <= 6){
		
	    if(grid[temp_x][temp_y] == player){
			counter++;
		}
		else{
			counter = 0;
		}
		if(counter == 4){
			return 1;
			}
		temp_x--;
		temp_y--;
	}

	return 0;
}

/*function that calls the horizontal,vertical and diagonal win check to see if the player has won the game or it should continue on*/
int check_win(char** grid, int x, int y, char player){
    return (check_win_diagonals(grid, x, y, player) || 
            check_win_horizontal(grid, x, y, player) || 
            check_win_vertical(grid, x, y, player));
}

/*function that takes as input the column choice of the user if it is a valid choice(in range and an integer) it checks the capacity of the column and places it in the lowest available slot then reprints the grid*/
int* update_grid(char** grid, int* capacities, char player){
    int choice;
    
    int* returnpos = (int*)malloc(2*sizeof(int));
    while(1){
        printf("Player %c, please choose a column: ", player);
        if(scanf("%d", &choice) != 1){
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

/*function that simulates playing the game by calling all functions*/
void start_game(){
    char** grid = malloc(ROWS * sizeof(char*));
    for(int i = 0; i < ROWS; i++){
        grid[i] = malloc(COLS * sizeof(char));
    }


    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            grid[i][j] = '.';
        }
    }

    int* column_capacity = calloc(COLS, sizeof(int));


    printf("Welcome to Connect Four!\n");
    printf("Player A: A\n");
    printf("Player B: B\n");
    display_grid(grid);
    

    int counter = 0;
    int flag = 0;
    while(flag == 0 && counter < 42){
        char player = (char)(65 + (counter % 2));
        int* position = update_grid(grid, column_capacity, player);
        display_grid(grid);
        if (check_win(grid, position[0], position[1], player) != 0){
            flag = (int)player;
        }
        counter++;
        free(position);
    }

    if(flag == 'A'){
        printf("Player A wins!\n");
    }

    else if(flag == 'B'){
        printf("Player B wins!\n");
    }

    else{
        printf("Draw!\n");
    }


    for(int i = 0; i < ROWS; i++){
        free(grid[i]);
    }
    free(grid);
    free(column_capacity);
}

/*main function that calls the start_game function to run the game*/
int main(){
    start_game();
    }
