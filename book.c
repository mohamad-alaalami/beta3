#include <stdint.h>
#include "main.h"

void book_partialKey3(uint64_t *key, int col, U64 current_position, U64 mask) {
    for (uint64_t pos = UINT64_C(1) << (col * (ROWS + 1)); pos & mask; pos <<= 1) {
        *key *= 3;
        if (pos & current_position) {
            *key += 1;
        } else {
            *key += 2;
        }
    }
    *key *= 3;
}

uint64_t book_key3(U64 playerA, U64 playerB, char playerToMove) {
    U64 current_position = (playerToMove == 'A') ? playerA : playerB;
    U64 mask = playerA | playerB;

    uint64_t key_forward = 0;
    for (int i = 0; i < COLS; i++) {
        book_partialKey3(&key_forward, i, current_position, mask);
    }

    uint64_t key_reverse = 0;
    for (int i = COLS; i--;) {
        book_partialKey3(&key_reverse, i, current_position, mask);
    }

    return (key_forward < key_reverse ? key_forward : key_reverse) / 3;
}
