#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

#define MEMLENGTH 4096  // memory pool size
#define ALIGNMENT 8
#define HEADER_SIZE sizeof(size_t)
#define MIN_CHUNK_SIZE 16

#define GET_SIZE(h) ((h) & ~(size_t)7)
#define IS_ALLOC(h) ((h) & (size_t)1)
#define MAKE_HEADER(s, a) ((s) | (a))

static union {
    char bytes[MEMLENGTH];
    double not_used;        
} heap;

static int initialized = 0;

// Detects memory leaks at the point of program termination
static void leak_detector(void) {
    size_t leaked = 0;
    int counter = 0;
    char *current = heap.bytes;

    // Traverses the heap to detect allocated, non-freed memory
    while (current < heap.bytes + MEMLENGTH) {
        size_t header = *((size_t *)current);
        
        if (header == 0) {
            break;
        }

        size_t chunkSize = GET_SIZE(header);  // Extracts chunk size (excludes the allocation bit)

        if (IS_ALLOC(header)) {  // Check if the block is allocated
            leaked += (chunkSize - HEADER_SIZE);
            counter++;  // Increment leak counter
        }

        current += chunkSize;  // Traverse to next block
    }

    if (leaked > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", leaked, counter);
    }
}

// Function to initialize the heap
static void initialize_heap(void) {
    atexit(leak_detector);  // Register leak detector to run at program exit
    *((size_t *)heap.bytes) = MAKE_HEADER(MEMLENGTH, 0);  // Set the first block size as the whole heap
    initialized = 1;  // Marks the heap as initialized
}

// Memory allocation function
void * mymalloc(size_t size, char *file, int line) {
    if (size == 0) {
        return NULL;  // Return NULL for zero-size allocation
    }
    
    if (!initialized) {
        initialize_heap();  // Initialize heap if not done already
    }

    // Ensure alignment to 8 bytes [cite: 101, 102]
    size_t paddedSize = (size + 7) & ~((size_t)7);
    size_t totalSize = paddedSize + HEADER_SIZE;
    
    char *current = heap.bytes;

    // Traverse the heap to find a suitable free block [cite: 87, 126]
    while (current < heap.bytes + MEMLENGTH) {
        size_t header = *((size_t *)current);
        size_t chunkSize = GET_SIZE(header);

        if (chunkSize == 0) break;

        if (!IS_ALLOC(header) && chunkSize >= totalSize) {
            size_t remaining = chunkSize - totalSize;

            // Split only if the remaining space can form a valid 16-byte chunk [cite: 88, 103]
            if (remaining >= MIN_CHUNK_SIZE) {
                *(size_t *)current = MAKE_HEADER(totalSize, 1);
                *(size_t *)(current + totalSize) = MAKE_HEADER(remaining, 0);
            } else {
                *(size_t *)current = MAKE_HEADER(chunkSize, 1);
            }
            return (void *)(current + HEADER_SIZE); // Return pointer to payload [cite: 128]
        }
        current += chunkSize;
    }

    // Spec-mandated error format [cite: 156]
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

// Memory freeing function
void myfree(void *ptr, char *file, int line) {
    if (ptr == NULL) {
        return;
    }
    
    // Ensure the pointer range is valid [cite: 159]
    if (ptr < (void *)heap.bytes || ptr >= (void *)(heap.bytes + MEMLENGTH)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }
    
    char *current = heap.bytes;
    size_t *target = NULL;
    int found_chunk = 0;

    // Locate the block containing the pointer [cite: 90]
    while (current < heap.bytes + MEMLENGTH) {
        size_t header = *(size_t *)current;
        size_t chunkSize = GET_SIZE(header);

        if (chunkSize == 0) break;

        if (current + HEADER_SIZE == (char *)ptr) {
            // Check for double free [cite: 166, 170]
            if (!IS_ALLOC(header)) {
                fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
                exit(2);
            }
            target = (size_t *)current;
            found_chunk = 1;
            break;
        }
        
        // Check if pointer is inside a chunk but not at the start [cite: 162, 164]
        if ((char *)ptr > current && (char *)ptr < current + chunkSize) {
            fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
            exit(2);
        }

        current += chunkSize;
    }

    if (!found_chunk) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    size_t chunkSize = GET_SIZE(*target);
    *target = MAKE_HEADER(chunkSize, 0);  // Mark as free [cite: 91]
    
    char *next = (char *)target + chunkSize;

    // Merge with the next block if it is free [cite: 92, 96]
    if (next < heap.bytes + MEMLENGTH) {
        size_t next_header = *((size_t *)next);
        size_t next_size = GET_SIZE(next_header);

        if (next_size != 0 && !IS_ALLOC(next_header)) {
            *target = MAKE_HEADER(chunkSize + next_size, 0);  // Merges blocks
            chunkSize = GET_SIZE(*target);
        }
    }
    
    char *iter = heap.bytes;
    size_t *previous = NULL;

    // Look for the previous adjacent free block [cite: 141]
    while (iter < (char *)target) {
        size_t currentHeader = *((size_t *)iter);
        size_t currentSize = GET_SIZE(currentHeader);
        if (iter + currentSize == (char *)target) {
            previous = (size_t *)iter;  // Find previous block
            break;
        }
        iter += currentSize;
    }
    
    // Merge with the previous block if it is free [cite: 67, 95]
    if (previous != NULL && !IS_ALLOC(*previous)) {
        size_t prev_size = GET_SIZE(*previous);
        *previous = MAKE_HEADER(prev_size + chunkSize, 0);  // Merges blocks
    }
}