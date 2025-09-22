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