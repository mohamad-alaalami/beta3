void display_grid(char **grid){
	for(int i = 0;i < 6;i++){
		for(int j = 0;j < 7;j++){
			printf("%c ", grid[i][j]);
		}
		printf("\n");
	}
	printf("1 2 3 4 5 6 7");
}