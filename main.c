#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

/*Function that displays the 6 by 7 board of the game*/
void display_grid(char **grid){
	for(int i = 0;i < ROWS;i++){
		for(int j = 0;j < COLS;j++){
			printf("%c  ", grid[i][j]);
		}
		printf("\n");
	}
	printf("1  2  3  4  5  6  7\n\n");
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
    //To save position to be used later
	int temp_x = x;
	int temp_y = y;
	int counter;

    /*Going to the lower leftmost availabe slot on the diagonal the player placed the
    char in*/ 
	while(temp_x >= 0 && temp_x < 5 && temp_y > 0 && temp_y < 6){
		temp_x++;
		temp_y--;
	}

    /*Setting counter to 0 and looping over the diagonal(going up then right) and incrementing counter if we view a char same as the one the player placed*/
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

    //Resetting the temp variables to the initial position
	temp_x = x;
	temp_y = y;

    /*Going to the lower rightmost availabe slot on the diagonal the player placed the
    char in*/ 
	while(temp_x >= 0 && temp_x < 5 && temp_y > 0 && temp_y < 6 ){
		temp_x++;
		temp_y++;
	}
    
    /*Setting counter to 0 and looping over the diagonal(going down then left) and incrementing counter if we view a char same as the one the player placed*/
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

/*Function that calls the horizontal,vertical and diagonal win check to see if the player has won the game or it should continue on*/
int check_win(char** grid, int x, int y, char player){
    return (check_win_diagonals(grid, x, y, player) || 
            check_win_horizontal(grid, x, y, player) || 
            check_win_vertical(grid, x, y, player));
}

/*
Function that updates the grid depending on the player/bot performing the move
*/
int* update_grid(char** grid, int* capacities, char player, int bot){
    if(bot == 0){
        return player_move(grid, capacities, player);
    }

    else if(bot == 1){
        return easy_move(grid, capacities, player);
    }

    else if(bot == 2){
        return medium_move(grid, capacities, player);
    }

    else if(bot == 3){
        return hard_move(grid, capacities, player);
    }

    else{
        return NULL;
    }
}

/*Function that simulates playing the game by calling all functions*/
void start_game(){
    srand(time(NULL));

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
    
    int bot;
    int random;
    char* A;
    char* B;
    {
        char choice;
        while(1){
            printf("Do you want to play against a player or bot?(p/b): ");
            scanf("%c", &choice);
            if(choice == 'p' || choice == 'P'){
                A = "Player A";
                B = "Player B";
                bot = 0;
                break;
            }
            
            else if(choice == 'b' || choice == 'B'){
                char difficulty;
                while(1){
                    printf("Select difficultuly: E(Easy)/M(Medium)/H(Hard): ");
                    scanf(" %c", &difficulty);
                    if(difficulty == 'e' || difficulty == 'E'){
                        bot = 1;
                        break;
                    }
                    
                    else if(difficulty == 'm' || difficulty == 'M'){
                    bot = 2;
                        break;
                    }
                    else if(difficulty == 'h' || difficulty == 'H'){
                    bot = 3;
                        break;
                    }
                }

                random = (rand() % 2);
                if(random){
                    A = "Player";
                    B = "Bot";
                }
                
                else{
                    A = "Bot";
                    B = "Player";
                }
                break;
            }
        }
    }

    printf("%s: A\n",A);
    printf("%s: B\n",B);
    display_grid(grid);

    int counter = 0;
    int flag = 0;
    while(flag == 0 && counter < 42){
        char player = (char)(65 + (counter % 2));
        int* position;
        if((random + counter) % 2 == 0){
            position = update_grid(grid, column_capacity, player, bot);
        }
        else{
            position = update_grid(grid, column_capacity, player, 0);
        }
        display_grid(grid);
        if (check_win(grid, position[0], position[1], player) != 0){
            flag = (int)player;
        }
        counter++;
        free(position);
    }

    if(flag == 'A'){
        printf("%s wins!\n",A);
    }

    else if(flag == 'B'){
        printf("%s wins!\n",B);
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
