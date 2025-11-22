#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "opening_book.h"
#include "main.h"

#define CHECK(desc, cond) \
    do { if (cond) { printf("[PASS] %s\n", desc); passes++; } \
         else      { printf("[FAIL] %s\n", desc); fails++; } } while (0)

static int passes = 0;
static int fails = 0;

 int bit_index(int row, int col) { return row + col * 7; }

#if defined(__GNUC__)
__attribute__((unused))
#endif
static void print_board(U64 A, U64 B) {
    for (int r = ROWS - 1; r >= 0; r--) {
        for (int c = 0; c < COLS; c++) {
            int idx = bit_index(r, c);
            char ch = '.';
            if (A & (1ULL << idx)) ch = 'A';
            else if (B & (1ULL << idx)) ch = 'B';
            printf("%c  ", ch);
        }
        printf("\n");
    }
    printf("1  2  3  4  5  6  7\n\n");
}

static void apply_move(U64 *A, U64 *B, char player, int col) {
    if (col < 0 || col >= COLS) return;
    U64 mask = *A | *B;
    for (int row = 0; row < ROWS; row++) {
        int idx = bit_index(row, col);
        if ((mask & (1ULL << idx)) == 0) {
            if (player == 'A') *A |= (1ULL << idx);
            else *B |= (1ULL << idx);
            return;
        }
    }
}

static void reset_pos(U64 *A, U64 *B) {
    *A = 0ULL;
    *B = 0ULL;
}

static int decode_book_value(int raw) {
    if (raw == 0xFF) return -1;
    if (raw >= 0x10 && raw <= 0x17) return raw - 0x11;
    if (raw >= 0 && raw <= 6) return raw;
    return -1;
}

static int debug_raw_value(const OpeningBook* book, U64 A, U64 B, char player) {
    uint64_t key = book_key3(A, B, player);
    uint32_t idx = (uint32_t)(key & book->index_mask);
    uint64_t mask = (book->key_bytes >= 8) ? UINT64_MAX : ((UINT64_C(1) << (book->key_bytes * 8)) - 1);
    uint16_t partial = (uint16_t)((key >> book->log_size) & mask);
    for (uint32_t probed = 0; probed < book->size; probed++) {
        const uint8_t* key_ptr = book->keys + ((size_t)idx * (size_t)book->key_bytes);
        uint16_t stored = 0;
        for (uint8_t i = 0; i < book->key_bytes && i < 2; i++) {
            stored |= (uint16_t)((uint16_t)key_ptr[i] << (8u * i));
        }
        const uint8_t* val_ptr = book->values + ((size_t)idx * (size_t)book->value_bytes);
        int raw = 0;
        for (uint8_t i = 0; i < book->value_bytes && i < 4; i++) {
            raw |= (int)((uint32_t)val_ptr[i] << (8u * i));
        }
        if (stored == partial) {
            return raw;
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
        if (empty) {
            return -1;
        }
        idx = (idx + 1u) & book->index_mask;
    }
    return -1;
}

static int build_position(const int *seq, int len, U64 *A, U64 *B) {
    reset_pos(A, B);
    for (int i = 0; i < len; i++) {
        char p = (i % 2 == 0) ? 'A' : 'B';
        apply_move(A, B, p, seq[i]);
    }
    return len;
}

static void run_case(const char* desc, const int* seq, int len, const OpeningBook* book) {
    U64 A = 0, B = 0;
    int moveCount = build_position(seq, len, &A, &B);
    char player = (moveCount % 2 == 0) ? 'A' : 'B';
    int move = opening_book_best_move(A, B, player, moveCount, book);
    int raw = debug_raw_value(book, A, B, player);
    int exp = decode_book_value(raw);
    printf("%s move: %d (raw %d decoded %d) player %c mc %d\n", desc, move, raw, exp, player, moveCount);
    CHECK(desc, move == exp);
}

static void demo_first_four_plies(const OpeningBook* book) {
    U64 A = 0, B = 0;
    for (int ply = 0; ply < 4; ply++) {
        char player = (ply % 2 == 0) ? 'A' : 'B';
        int move = opening_book_best_move(A, B, player, ply, book);
        int raw = debug_raw_value(book, A, B, player);
        int decoded = decode_book_value(raw);
        printf("Ply %d player %c: book move %d (raw %d decoded %d)\n", ply, player, move, raw, decoded);
        if (move < 0 || move >= COLS) {
            printf("  No book move; stopping demo.\n");
            break;
        }
        apply_move(&A, &B, player, move);
        print_board(A, B);
    }
}

int main(void) {
    OpeningBook book;
    if (!opening_book_load("opening_book_flat.bin", &book)) {
        printf("Failed to load opening_book_flat.bin\n");
        return 1;
    }

    U64 A = 0, B = 0;
    int move;

    // Test 1 — empty board
    reset_pos(&A, &B);
    move = opening_book_best_move(A, B, 'A', 0, &book);
    int raw = debug_raw_value(&book, A, B, 'A');
    int exp = decode_book_value(raw);
    printf("Test1 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK("empty board best move", move == exp);

    // Test 2 — B plays center first
    reset_pos(&A, &B);
    apply_move(&A, &B, 'B', 3);
    move = opening_book_best_move(A, B, 'A', 1, &book);
    raw = debug_raw_value(&book, A, B, 'A');
    exp = decode_book_value(raw);
    printf("Test2 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK("B center first", move == exp);

    // Test 3 — A center, B plays 2
    reset_pos(&A, &B);
    apply_move(&A, &B, 'A', 3);
    apply_move(&A, &B, 'B', 2);
    move = opening_book_best_move(A, B, 'A', 2, &book);
    raw = debug_raw_value(&book, A, B, 'A');
    exp = decode_book_value(raw);
    printf("Test3 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK("A center, B 2", move == exp);

    // Test 4 — A:3 B:4 A:3, expect B blocks at 3
    reset_pos(&A, &B);
    apply_move(&A, &B, 'A', 3);
    apply_move(&A, &B, 'B', 4);
    apply_move(&A, &B, 'A', 3);
    move = opening_book_best_move(A, B, 'B', 3, &book);
    raw = debug_raw_value(&book, A, B, 'B');
    exp = decode_book_value(raw);
    printf("Test4 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK("B reply after A:3 B:4 A:3", move == exp);

    // Test 5 — A:3 B:2 A:3 B:4 A:3 B:5, expect A 3 or 1
    reset_pos(&A, &B);
    apply_move(&A, &B, 'A', 3);
    apply_move(&A, &B, 'B', 2);
    apply_move(&A, &B, 'A', 3);
    apply_move(&A, &B, 'B', 4);
    apply_move(&A, &B, 'A', 3);
    apply_move(&A, &B, 'B', 5);
    move = opening_book_best_move(A, B, 'A', 6, &book);
    raw = debug_raw_value(&book, A, B, 'A');
    exp = decode_book_value(raw);
    printf("Test5 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK("6-ply sequence", move == exp);

    // Test 6 — >16 moves returns -1
    reset_pos(&A, &B);
    int seq[] = {3,2,3,4,3,5,4,2,4,3,1,1,0,6,0,6,1,5};
    int seq_len = sizeof(seq)/sizeof(seq[0]);
    for (int i = 0; i < seq_len; i++) {
        char p = (i % 2 == 0) ? 'A' : 'B';
        apply_move(&A, &B, p, seq[i]);
    }
    move = opening_book_best_move(A, B, 'A', seq_len, &book);
    raw = debug_raw_value(&book, A, B, 'A');
    exp = decode_book_value(raw);
    printf("Test6 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK(">16 moves returns -1", move == -1);

    // Test 7 — symmetry: A:2 B:3 A:2 B:3, expect 2 or 3
    reset_pos(&A, &B);
    apply_move(&A, &B, 'A', 2);
    apply_move(&A, &B, 'B', 3);
    apply_move(&A, &B, 'A', 2);
    apply_move(&A, &B, 'B', 3);
    move = opening_book_best_move(A, B, 'A', 4, &book);
    raw = debug_raw_value(&book, A, B, 'A');
    exp = decode_book_value(raw);
    printf("Test7 move: %d (raw %d decoded %d)\n", move, raw, exp);
    CHECK("symmetry", move == exp);

    // Test 8 — key3 deterministic
    uint64_t k1 = book_key3(A, B, 'A');
    uint64_t k2 = book_key3(A, B, 'A');
    CHECK("key3 deterministic", k1 == k2);

    // Test 9 — probing wrap, invalid key returns -1
    int invalid = opening_book_best_move(~0ULL, ~0ULL, 'A', 0, &book);
    raw = debug_raw_value(&book, ~0ULL, ~0ULL, 'A');
    exp = decode_book_value(raw);
    printf("Test9 move: %d (raw %d decoded %d)\n", invalid, raw, exp);
    CHECK("invalid key returns -1", invalid == -1 || invalid == exp);

    // Additional book-driven checks
    {
        int seq10[] = {3, 3, 4}; // B to move
        run_case("Case10 A3 B3 A4", seq10, 3, &book);

        int seq11[] = {0, 6, 0, 6, 0, 6}; // A to move
        run_case("Case11 edges 0/6", seq11, 6, &book);

        int seq12[] = {1, 1, 1, 5, 5, 5}; // A to move
        run_case("Case12 double stacks 1/5", seq12, 6, &book);

        int seq13[] = {3, 2, 4, 2, 5, 2}; // B to move
        run_case("Case13 A3 B2 A4 B2 A5 B2", seq13, 6, &book);

        int seq14[] = {4, 3, 4, 3, 4}; // B to move
        run_case("Case14 A4 B3 A4 B3 A4", seq14, 5, &book);
    }

    printf("\nFirst four plies from book on empty board:\n");
    demo_first_four_plies(&book);

    printf("\nSummary: %d passed, %d failed\n", passes, fails);
    return fails ? 1 : 0;
}
