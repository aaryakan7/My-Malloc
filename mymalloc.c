#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define ALIGNMENT 8
#define HEADER_SIZE ALIGNMENT
#define MIN_CHUNK_SIZE (HEADER_SIZE + ALIGNMENT)
#define GET_SIZE(header) ((header) & ~(size_t)(ALIGNMENT - 1))
#define IS_ALLOC(header) ((header) & (size_t)1)
#define MAKE_HEADER(size, allocated) ((size) | (allocated))

static union {
    char bytes[MEMLENGTH];
    double alignment;
} heap;

static int initialized = 0;

static void leak_detector(void) {
    size_t leaked_bytes = 0;
    int leaked_objects = 0;
    char *current = heap.bytes;

    while (current < heap.bytes + MEMLENGTH) {
        size_t header = *(size_t *)current;
        size_t chunk_size = GET_SIZE(header);

        if (chunk_size < MIN_CHUNK_SIZE || current + chunk_size > heap.bytes + MEMLENGTH) {
            break;
        }
        if (IS_ALLOC(header)) {
            leaked_bytes += chunk_size - HEADER_SIZE;
            leaked_objects++;
        }
        current += chunk_size;
    }

    if (leaked_bytes > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", leaked_bytes, leaked_objects);
    }
}

static void initialize_heap(void) {
    *(size_t *)heap.bytes = MAKE_HEADER(MEMLENGTH, 0);
    atexit(leak_detector);
    initialized = 1;
}

static void ensure_initialized(void) {
    if (!initialized) {
        initialize_heap();
    }
}

static void allocation_error(size_t size, char *file, int line) {
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
}

static void invalid_free(char *file, int line) {
    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
}

void *mymalloc(size_t size, char *file, int line) {
    ensure_initialized();
    if (size == 0) {
        return NULL;
    }

    if (size > MEMLENGTH - HEADER_SIZE) {
        allocation_error(size, file, line);
        return NULL;
    }

    size_t padded_size = (size + ALIGNMENT - 1) & ~(size_t)(ALIGNMENT - 1);
    size_t total_size = padded_size + HEADER_SIZE;

    for (char *current = heap.bytes; current < heap.bytes + MEMLENGTH;) {
        size_t header = *(size_t *)current;
        size_t chunk_size = GET_SIZE(header);

        if (chunk_size < MIN_CHUNK_SIZE || current + chunk_size > heap.bytes + MEMLENGTH) {
            break;
        }
        if (!IS_ALLOC(header) && chunk_size >= total_size) {
            size_t remaining = chunk_size - total_size;
            if (remaining >= MIN_CHUNK_SIZE) {
                *(size_t *)current = MAKE_HEADER(total_size, 1);
                *(size_t *)(current + total_size) = MAKE_HEADER(remaining, 0);
            } else {
                *(size_t *)current = MAKE_HEADER(chunk_size, 1);
            }
            return current + HEADER_SIZE;
        }
        current += chunk_size;
    }

    allocation_error(size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    ensure_initialized();
    if (ptr == NULL) {
        return;
    }

    uintptr_t address = (uintptr_t)ptr;
    uintptr_t heap_start = (uintptr_t)heap.bytes;
    uintptr_t heap_end = heap_start + MEMLENGTH;
    if (address < heap_start + HEADER_SIZE || address >= heap_end) {
        invalid_free(file, line);
    }

    char *current = heap.bytes;
    size_t *target = NULL;
    while (current < heap.bytes + MEMLENGTH) {
        size_t header = *(size_t *)current;
        size_t chunk_size = GET_SIZE(header);

        if (chunk_size < MIN_CHUNK_SIZE || current + chunk_size > heap.bytes + MEMLENGTH) {
            invalid_free(file, line);
        }
        if (current + HEADER_SIZE == (char *)ptr) {
            if (!IS_ALLOC(header)) {
                invalid_free(file, line);
            }
            target = (size_t *)current;
            break;
        }
        if (address > (uintptr_t)current && address < (uintptr_t)(current + chunk_size)) {
            invalid_free(file, line);
        }
        current += chunk_size;
    }
    if (target == NULL) {
        invalid_free(file, line);
    }

    size_t chunk_size = GET_SIZE(*target);
    *target = MAKE_HEADER(chunk_size, 0);

    char *next = (char *)target + chunk_size;
    if (next < heap.bytes + MEMLENGTH) {
        size_t next_header = *(size_t *)next;
        size_t next_size = GET_SIZE(next_header);
        if (next_size != 0 && !IS_ALLOC(next_header)) {
            chunk_size += next_size;
            *target = MAKE_HEADER(chunk_size, 0);
        }
    }

    for (char *previous = heap.bytes; previous < (char *)target;) {
        size_t previous_header = *(size_t *)previous;
        size_t previous_size = GET_SIZE(previous_header);
        if (previous_size == 0 || previous_size < MIN_CHUNK_SIZE) {
            break;
        }
        if (previous + previous_size == (char *)target) {
            if (!IS_ALLOC(previous_header)) {
                *(size_t *)previous = MAKE_HEADER(previous_size + chunk_size, 0);
            }
            break;
        }
        previous += previous_size;
    }
}
