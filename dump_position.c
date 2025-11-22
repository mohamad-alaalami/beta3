#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "opening_book.h"

static void apply_move(U64 *A, U64 *B, char player, int col) {
    U64 mask = *A | *B;
    int h = 0;
    for (int row = 0; row < ROWS; row++) {
        int idx = row + col * 7;
        if ((mask & (1ULL << idx)) == 0) {
            h = row;
            break;
        }
    }
    if (h >= ROWS) return;
    int idx = h + col * 7;
    if (player == 'A') *A |= (1ULL << idx);
    else *B |= (1ULL << idx);
}

int main(void) {
    U64 A = 0, B = 0;
    apply_move(&A, &B, 'A', 3);
    apply_move(&A, &B, 'B', 3);
    apply_move(&A, &B, 'A', 2);
    apply_move(&A, &B, 'B', 4);

    printf("playerA = 0x%llx\n", (unsigned long long)A);
    printf("playerB = 0x%llx\n", (unsigned long long)B);
    printf("key3 = %llu\n", (unsigned long long)book_key3(A, B, 'A'));
    return 0;
}
