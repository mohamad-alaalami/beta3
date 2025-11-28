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
