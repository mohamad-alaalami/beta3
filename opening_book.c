#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "opening_book.h"
#include "main.h"

int opening_book_load(const char* filename, OpeningBook* book) {
    if (!filename || !book) {
        return 0;
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        return 0;
    }

    book->keys = NULL;
    book->values = NULL;

    if (fread(&book->size, sizeof(uint32_t), 1, f) != 1 ||
        fread(&book->log_size, sizeof(uint8_t), 1, f) != 1 ||
        fread(&book->key_bytes, sizeof(uint8_t), 1, f) != 1 ||
        fread(&book->value_bytes, sizeof(uint8_t), 1, f) != 1 ||
        fread(&book->reserved, sizeof(uint8_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    if (book->log_size >= 32 || book->key_bytes == 0 || book->value_bytes == 0) {
        fclose(f);
        return 0;
    }

    book->index_mask = (1u << book->log_size) - 1u;

    size_t keys_size = (size_t)book->size * (size_t)book->key_bytes;
    size_t values_size = (size_t)book->size * (size_t)book->value_bytes;

    book->keys = (uint8_t*)malloc(keys_size);
    if (!book->keys) {
        fclose(f);
        return 0;
    }

    book->values = (uint8_t*)malloc(values_size);
    if (!book->values) {
        free(book->keys);
        book->keys = NULL;
        fclose(f);
        return 0;
    }

    if (fread(book->keys, 1, keys_size, f) != keys_size) {
        free(book->keys);
        free(book->values);
        book->keys = NULL;
        book->values = NULL;
        fclose(f);
        return 0;
    }

    if (fread(book->values, 1, values_size, f) != values_size) {
        free(book->keys);
        free(book->values);
        book->keys = NULL;
        book->values = NULL;
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

int opening_book_best_move(U64 playerA, U64 playerB, char playerToMove,
                           int moveCount, const OpeningBook* book) {
    if (!book || !book->keys || !book->values) {
        return -1;
    }

    if (moveCount > 16) {
        return -1;
    }

    uint64_t key = book_key3(playerA, playerB, playerToMove);
    uint32_t idx = (uint32_t)(key & book->index_mask);

    uint64_t mask;
    if (book->key_bytes >= 8) {
        mask = UINT64_MAX;
    } else {
        mask = (UINT64_C(1) << (book->key_bytes * 8)) - 1;
    }

    uint16_t partial = (uint16_t)((key >> book->log_size) & mask);

    for (uint32_t probed = 0; probed < book->size; probed++) {
        const uint8_t* key_ptr = book->keys + ((size_t)idx * (size_t)book->key_bytes);
        uint16_t stored = 0;
        for (uint8_t i = 0; i < book->key_bytes && i < 2; i++) {
            stored |= (uint16_t)((uint16_t)key_ptr[i] << (8u * i));
        }

        const uint8_t* val_ptr = book->values + ((size_t)idx * (size_t)book->value_bytes);
        int value_raw = 0;
        for (uint8_t i = 0; i < book->value_bytes && i < 4; i++) {
            value_raw |= (int)((uint32_t)val_ptr[i] << (8u * i));
        }
        int value;
        if (value_raw == 0xFF) {
            value = -1;
        } else if (value_raw >= 0x10 && value_raw <= 0x17) {
            value = value_raw - 0x11; // book stores 1-based moves offset by 0x10
        } else if (value_raw <= 6) {
            value = value_raw; // already 0-based
        } else {
            value = -1; // unknown encoding, treat as absent
        }

        if (stored == partial) {
            return value;
        }

        bool empty = true;
        for (uint8_t i = 0; i < book->key_bytes; i++) {
            if (key_ptr[i] != 0) {
                empty = false;
                break;
            }
        }
        if (empty) {
            for (uint8_t i = 0; i < book->value_bytes; i++) {
                if (val_ptr[i] != 0) {
                    empty = false;
                    break;
                }
            }
        }
        if (empty) return -1;

        idx = (idx + 1u) & book->index_mask;
    }

    return -1;
}
