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