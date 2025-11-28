#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "opening_book.h"

OpeningBook BOOK;

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
    (void)y;
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
int* update_grid(char** grid, int* capacities, int counter, char player, int bot, const char* colstring){
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
        return hard_move(grid, capacities, counter, player, colstring);
    }

    else{
        return NULL;
    }
}
void game_starter(char** A, char** B, int* start_bot, int* bot){
    *start_bot = 0;
    char choice;
    while(1){
        printf("Do you want to play against a player or bot?(p/b): ");
        if (scanf(" %c", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        if(choice == 'p' || choice == 'P'){
            *A = "Player A";
            *B = "Player B";
            *bot = 0;
            *start_bot = 0;
            break;
        }
        
        else if(choice == 'b' || choice == 'B'){
            char difficulty;
            while(1){
                printf("Select difficultuly: E(Easy)/M(Medium)/H(Hard): ");
                if (scanf(" %c", &difficulty) != 1) {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF) {}
                    continue;
                }
                if(difficulty == 'e' || difficulty == 'E'){
                    *bot = 1;
                    break;
                }
                
                else if(difficulty == 'm' || difficulty == 'M'){
                    *bot = 2;
                    break;
                }
                else if(difficulty == 'h' || difficulty == 'H'){
                    *bot = 3;
                    break;
                }
            }

            char starter;
            while (1) {
                printf("Who starts first? Player (p) or Bot (b): ");
                if (scanf(" %c", &starter) != 1) {
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF) {}
                    continue;
                }
                if (starter == 'p' || starter == 'P') {
                    *start_bot = 0;
                    *A = "Player";
                    *B = "Bot";
                    break;
                } else if (starter == 'b' || starter == 'B') {
                    *start_bot = 1;
                    *A = "Bot";
                    *B = "Player";
                    break;
                }
            }
            break;
        }
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
    int start_bot;
    char* A;
    char* B;
    game_starter(&A,&B,&start_bot,&bot);

    printf("%s: A\n",A);
    printf("%s: B\n",B);
    display_grid(grid);

    int counter = 0;
    char colstring[ROWS * COLS + 1] = {0};
    int col_len = 0;
    int flag = 0;
    while(flag == 0 && counter < 42){
        char player = (char)(65 + (counter % 2));
        int* position;
        int bot_turn = 0;
        if (bot != 0) {
            bot_turn = ((counter % 2 == 0) ? start_bot : !start_bot);
        }
        if(bot_turn){
            position = update_grid(grid, column_capacity, counter, player, bot, colstring);
        }
        else{
            position = update_grid(grid, column_capacity, counter, player, 0, colstring);
        }
        display_grid(grid);
        if (check_win(grid, position[0], position[1], player) != 0){
            flag = (int)player;
        }
        if (col_len < ROWS * COLS) {
            colstring[col_len++] = (char)('1' + position[1]);
            colstring[col_len] = '\0';
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

static void apply_move_bitboards(U64* A, U64* B, int* heights, int col, char player) {
    if (col < 0 || col >= COLS) return;
    if (heights[col] >= ROWS) return;
    int row = heights[col];
    int idx = bit_index(row, col);
    if (player == 'A') {
        *A |= (1ULL << idx);
    } else {
        *B |= (1ULL << idx);
    }
    heights[col]++;
}

void run_opening_book_integration_test(void) {
    U64 A = 0, B = 0;
    int heights[COLS] = {0};

    // ply 0: empty, A to move
    uint64_t k0 = book_key3(A, B, 'A');
    int m0 = opening_book_lookup_key(k0, &BOOK);
    printf("PLY 0 | turn=A | key3=%llu | move=%d\n", (unsigned long long)k0, m0);

    // ply 1: A plays 3, B to move
    apply_move_bitboards(&A, &B, heights, 3, 'A');
    uint64_t k1 = book_key3(A, B, 'B');
    int m1 = opening_book_lookup_key(k1, &BOOK);
    printf("PLY 1 | turn=B | key3=%llu | move=%d\n", (unsigned long long)k1, m1);

    // ply 2: B plays 3, A to move
    apply_move_bitboards(&A, &B, heights, 3, 'B');
    uint64_t k2 = book_key3(A, B, 'A');
    int m2 = opening_book_lookup_key(k2, &BOOK);
    printf("PLY 2 | turn=A | key3=%llu | move=%d\n", (unsigned long long)k2, m2);

    // ply 3: A plays 2, B to move
    apply_move_bitboards(&A, &B, heights, 2, 'A');
    uint64_t k3 = book_key3(A, B, 'B');
    int m3 = opening_book_lookup_key(k3, &BOOK);
    printf("PLY 3 | turn=B | key3=%llu | move=%d\n", (unsigned long long)k3, m3);
}

/*main function that calls the start_game function to run the game*/
int main(int argc, char** argv){
    if (!opening_book_load("book_10ply.bin", &BOOK)) {
        printf("Failed to load opening book.\n");
        return 1;
    }

    if (argc > 1 && strcmp(argv[1], "--testbook") == 0) {
        run_opening_book_integration_test();
        return 0;
    }

    start_game();
}
