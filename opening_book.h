#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <stdint.h>
#include "main.h"

typedef struct {
    uint32_t size;        // number of buckets (power of 2)
    uint32_t index_mask;  // size - 1
    uint8_t  key_bytes;   // bytes per key (should be 8)
    uint8_t  val_bytes;   // bytes per value (should be 1)
    uint16_t _padding;

    uint64_t* keys;       // array[size] of full 64-bit key3 values
    uint8_t*  values;     // array[size] of best moves (255 = empty)
} OpeningBook;

uint64_t book_key3(U64 playerA, U64 playerB, char playerToMove);

int opening_book_lookup_key(uint64_t key, const OpeningBook* book);
int opening_book_load(const char* filename, OpeningBook* book);
void opening_book_free(OpeningBook* book);
int opening_book_best_move(U64 playerA, U64 playerB, char playerToMove,
                           int moveCount, const OpeningBook* book);

#endif
