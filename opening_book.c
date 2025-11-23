#include "opening_book.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opening_book_lookup_key(uint64_t key, const OpeningBook* book) {
    if (!book || !book->keys || !book->values || book->size == 0)
        return -1;

    uint32_t mask = book->index_mask;
    uint32_t i = (uint32_t)(key & mask);

    for (uint32_t probed = 0; probed < book->size; probed++) {
        uint64_t k = book->keys[i];
        uint8_t  v = book->values[i];

        if (k == key)
            return (int)v;

        // Empty bucket: key=0 && value=255
        if (k == 0 && v == 255)
            return -1;

        i = (i + 1u) & mask;
    }
    return -1;
}

int opening_book_load(const char* filename, OpeningBook* book) {
    memset(book, 0, sizeof(*book));

    FILE* f = fopen(filename, "rb");
    if (!f) return 0;

    uint8_t key_bytes = 0, val_bytes = 0, reserved = 0;

    // Read header
    if (fread(&book->size,       4, 1, f) != 1) goto fail;
    if (fread(&book->index_mask, 4, 1, f) != 1) goto fail;
    if (fread(&key_bytes,        1, 1, f) != 1) goto fail;
    if (fread(&val_bytes,        1, 1, f) != 1) goto fail;
    if (fread(&reserved,         1, 1, f) != 1) goto fail;

    // table size must be power of 2 and mask must equal size-1
    if ((book->size & (book->size - 1)) != 0)
        goto fail;
    if (book->index_mask != book->size - 1)
        goto fail;
    if (key_bytes != 8 || val_bytes != 1)
        goto fail;

    // allocate memory
    book->keys   = (uint64_t*)malloc(sizeof(uint64_t) * book->size);
    book->values = (uint8_t*) malloc(sizeof(uint8_t)  * book->size);

    if (!book->keys || !book->values)
        goto fail;

    // read keys
    if (fread(book->keys, sizeof(uint64_t), book->size, f) != book->size)
        goto fail;

    // read values
    if (fread(book->values, sizeof(uint8_t),  book->size, f) != book->size)
        goto fail;

    fclose(f);
    return 1;

fail:
    if (book->keys)   free(book->keys);
    if (book->values) free(book->values);
    memset(book, 0, sizeof(*book));
    if (f) fclose(f);
    return 0;
}

void opening_book_free(OpeningBook* book) {
    if (!book) return;
    free(book->keys);
    free(book->values);
    memset(book, 0, sizeof(*book));
}

int opening_book_best_move(U64 playerA, U64 playerB, char playerToMove,
                           int moveCount, const OpeningBook* book) {
    (void)moveCount; // book itself encodes available plies; caller may still gate by depth
    uint64_t key = book_key3(playerA, playerB, playerToMove);
    int mv = opening_book_lookup_key(key, book);
    return mv;
}
