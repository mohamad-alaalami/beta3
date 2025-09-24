int check_win_diagonals(char **grid, int x, int y, char player){
	int temp_x = x;
	int temp_y = y;
	int counter;

	while(temp_x > 0 && temp_x < 5 && temp_y > 0 && temp_y < 6){
		temp_x++;
		temp_y--;
	}

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
	temp_x = x;
	temp_y = y;

	while(temp_x > 0 && temp_x < 5 && temp_y > 0 && temp_y < 6 ){
		temp_x++;
		temp_y++;
		
	}

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