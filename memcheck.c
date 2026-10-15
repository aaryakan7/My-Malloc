#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

static int check_data(const unsigned char *memory, size_t size, unsigned char value) {
    for (size_t i = 0; i < size; i++) {
        if (memory[i] != value) {
            fprintf(stderr, "data changed at byte %zu\n", i);
            return 0;
        }
    }
    return 1;
}

static int check_layout(void) {
    unsigned char *first = malloc(31);
    unsigned char *second = malloc(47);
    if (first == NULL || second == NULL || first == second ||
        (uintptr_t)first % 8 != 0 || (uintptr_t)second % 8 != 0) {
        fprintf(stderr, "allocation, alignment, or separation check failed\n");
        return 0;
    }

    memset(first, 0x3c, 31);
    memset(second, 0xa7, 47);
    if (!check_data(first, 31, 0x3c) || !check_data(second, 47, 0xa7)) {
        return 0;
    }
    free(first);
    free(second);
    return 1;
}

static int check_reuse_and_coalescing(void) {
    void *first = malloc(1000);
    void *second = malloc(1000);
    void *third = malloc(1000);
    if (first == NULL || second == NULL || third == NULL) {
        fprintf(stderr, "could not allocate test blocks\n");
        return 0;
    }
    free(first);
    free(second);
    free(third);

    void *combined = malloc(4000);
    if (combined == NULL) {
        fprintf(stderr, "freed neighboring blocks were not coalesced\n");
        return 0;
    }
    free(combined);
    return 1;
}

static int check_capacity(void) {
    void *whole_pool = malloc(4088);
    if (whole_pool == NULL) {
        fprintf(stderr, "maximum aligned payload did not fit\n");
        return 0;
    }
    free(whole_pool);

    if (malloc(SIZE_MAX) != NULL) {
        fprintf(stderr, "oversized request was accepted\n");
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    if (argc == 2) {
        int local_value = 0;
        if (strcmp(argv[1], "outside-free") == 0) {
            free(&local_value);
        } else if (strcmp(argv[1], "interior-free") == 0) {
            unsigned char *memory = malloc(16);
            if (memory != NULL) {
                free(memory + 1);
            }
        } else if (strcmp(argv[1], "double-free") == 0) {
            void *memory = malloc(16);
            free(memory);
            free(memory);
        } else {
            fprintf(stderr, "usage: %s [outside-free|interior-free|double-free]\n", argv[0]);
            return 2;
        }
        fprintf(stderr, "invalid free unexpectedly returned\n");
        return 1;
    }
    if (argc != 1) {
        fprintf(stderr, "usage: %s [outside-free|interior-free|double-free]\n", argv[0]);
        return 2;
    }

    if (!check_layout() || !check_reuse_and_coalescing() || !check_capacity()) {
        return 1;
    }
    puts("allocator checks passed");
    return 0;
}
