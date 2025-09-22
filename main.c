#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int check_win_vertical(char** grid, int x, int y, char player)
{
    if(y < 4)
    {
        return 0;
    }
    int counter = 1;
    for(int i = y-1; i >= 0; i++)
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


int check_win_horizontal(char** grid, int x, int y, char player)
{
    int counter = 0;
    for(int i = 0; i < COLS; i++)
    {
        if(grid[i][y] == player)
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


void start_game()
{
    char** grid = malloc(ROWS * sizeof(char*));
    for(int i = 0; i < COLS; i++)
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

    int* column_capacity = malloc(ROWS * sizeof(int));
    column_capacity[0] = 0;

    printf("Welcome to Connect Four!\n");
    printf("Player A: A\n");
    printf("Player B: B\n");
    display_grid(grid);
    

    int counter = 0;
    int flag = 0;
    while(flag == 0 && counter < 47)
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


    for(int i = 0; i < COLS; i++)
    {
        free(grid[i]);
    }
    free(grid);
    free(column_capacity);
}