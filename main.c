#include <stdio.h>
#include <stdlib.h>
#include "main.h"


void display_grid(char **grid){
	for(int i = 0;i < ROWS;i++){
		for(int j = 0;j < COLS;j++){
			printf("%c  ", grid[i][j]);
		}
		printf("\n");
	}
	printf("1  2  3  4  5  6  7\n");
}


int check_win_vertical(char** grid, int x, int y, char player)
{
    if(x >= 3)
    {
        return 0;
    }
    int counter = 0;
    for(int i = x; i <= 5; i++)
    {
        if(grid[i][y] == player)
        {
            counter++;
        }
        else
        {
            return 0;
        }
        if(counter == 4)
        {
            return 1;
        }
    }
    return 0;
}

int check_win_horizontal(char** grid, int x, int y, char player)
{
    int counter = 0;
    for(int i = 0; i < COLS; i++)
    {
        if(grid[x][i] == player)
        {
            counter++;
            if(counter == 4)
            {
                return 1;
            }
        }
        else
        {
            counter = 0;
        }
    }
    return 0;
}

int check_win_diagonals(char **grid, int x, int y, char player){
    int temp_x = x;
	int temp_y = y;
	int counter = 0;
    
	for(int z = y;z < 6;z++){
        temp_x--;
		temp_y++;
	}

	while(temp_x >= 0 && temp_x < 6 && temp_y >= 0 && temp_y < 7){
		if(grid[temp_x][temp_y] == player){
            counter++;
		}
		if(counter == 4){
            return 1;
		}
        
		temp_x++;
		temp_y--;
	}
    
	temp_x = x;
	temp_y = y;
	counter = 0;
    
	for(int z = y;z < 6;z++){
        temp_x--;
		temp_y--;
	}

	while(temp_x >= 0 && temp_x < 6 && temp_y >= 0 && temp_y < 7){
		if(grid[temp_x][temp_y] == player){
            counter++;
		}
		if(counter == 4){
            return 1;
		}

		temp_x++;
		temp_y++;
	}
    
	return 0;
}

int check_win(char** grid, int x, int y, char player){
    return (check_win_diagonals(grid, x, y, player) || check_win_horizontal(grid, x, y, player) || check_win_vertical(grid, x, y, player));

}


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

            else {
                printf("column is full.\n");
            }
        }
        else{printf("invalid column.\n");}

    }
}


void start_game()
{
    char** grid = malloc(ROWS * sizeof(char*));
    for(int i = 0; i < ROWS; i++)
    {
        grid[i] = malloc(COLS * sizeof(char));
    }


    for(int i = 0; i < ROWS; i++)
    {
        for(int j = 0; j < COLS; j++)
        {
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
    while(flag == 0 && counter < 42)
    {
        char player = (char)(65 + (counter % 2));
        int* position = update_grid(grid, column_capacity, player);
        display_grid(grid);
        if (check_win(grid, position[0], position[1], player) != 0)
        {
            flag = (int)player;
        }
        counter++;
        free(position);
    }

    if(flag == 'A')
    {
        printf("Player A wins!\n");
    }

    else if(flag == 'B')
    {
        printf("Player B wins!\n");
    }

    else
    {
        printf("Draw!\n");
    }


    for(int i = 0; i < ROWS; i++)
    {
        free(grid[i]);
    }
    free(grid);
    free(column_capacity);
}

int main(){start_game();}