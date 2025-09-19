#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

#define MEMLENGTH 4096  // memory pool size

static union {
    char bytes[MEMLENGTH];
    double not_used;        
} heap;

static int initialized = 0; 

// Detects memory leaks at the point of program termination
static void leak_detector(void) 
{
    size_t leaked = 0;
    int counter = 0;
    char *current = heap.bytes;

    // Traverses the heap to detect allocated, non-freed memory
    while (current < heap.bytes + MEMLENGTH) 
    {
        size_t header = *((size_t *)current);
        
        if (header == 0) 
        {
            break;
        }

        size_t chunkSize = header & ~(size_t)1;  // Extracts chunk size (excludes the allocation bit)

        if (header & 1)  // Check if the block is allocated
        {
            leaked += (chunkSize - sizeof(size_t));
            counter++;  // Increment leak counter
        }

        current += chunkSize;  // Traverse to next block
    }

    if (leaked > 0) 
    {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", leaked, counter);
    }
}

// Function to initialize the heap
static void initialize_heap(void) 
{
    atexit(leak_detector);  // Register leak detector to run at program exit
    *((size_t *)heap.bytes) = MEMLENGTH;  // Set the first block size as the whole heap
    initialized = 1;  // Marks the heap as initialized
}

// Memory allocation function
void * mymalloc(size_t size, char *file, int line) 
{
    if (!initialized) 
    {
        initialize_heap();  // Initialize heap if not done already
    }

    // Ensure alignment to 8 bytes
    size_t paddedSize = (size + 7) & ~((size_t)7);
    size_t totalSize = paddedSize + sizeof(size_t);
    
    char *current = heap.bytes;

    // Traverse the heap to find a suitable free block
    while (current < heap.bytes + MEMLENGTH) 
    {
        size_t header = *((size_t *)current);
        if (header == 0) {
            break;
        }

        size_t chunkSize = header & ~(size_t)1;

        if (!(header & 1) && chunkSize >= totalSize)
        {
            size_t remaining = chunkSize - totalSize;

            if (remaining >= 16)  //Split if enough space remains
            {
                *((size_t *)current) = totalSize | 1;
                char *next = current + totalSize;
                *((size_t *)next) = remaining; 
            } 
            else 
            {
                *((size_t *)current) = chunkSize | 1;  // Allocate the entire block
            }

            return (void *)(current + sizeof(size_t));  // Return pointer to the user's memory
        }

        current += chunkSize;
    }

    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

// Memory freeing function
void myfree(void *ptr, char *file, int line) 
{
    if (ptr == NULL) 
    {
        return;
    }
    
    // Ensure the pointer range is valid
    if (ptr < (void *)heap.bytes || ptr >= (void *)(heap.bytes + MEMLENGTH)) 
    {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }
    
    char *current = heap.bytes;
    size_t *target = NULL;

    // Locate the block containing the pointer
    while (current < heap.bytes + MEMLENGTH) 
    {
        size_t header = *((size_t *)current);

        if (header == 0) 
        {
            break;
        }

        if (current + sizeof(size_t) == (char *)ptr) 
        {
            target = (size_t *)current;  //Find the target block
            break;
        }

        current += (header & ~(size_t)1);  //Move to the next block
    }

    if (target == NULL) 
    {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    if (((*target) & 1) == 0)
    {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    size_t chunkSize = (*target) & ~(size_t)1;
    *target = chunkSize;  // Mark as free
    
    char *next = (char *)target + chunkSize;

    // Merge with the next block if it is free
    if (next < heap.bytes + MEMLENGTH) 
    {
        size_t next_header = *((size_t *)next);

        if (next_header != 0 && (next_header & 1) == 0) 
        {
            size_t next_size = next_header & ~(size_t)1;
            *target = chunkSize + next_size;  // Merges blocks
            chunkSize = *target;
        }
    }
    
    char *iter = heap.bytes;
    size_t *previous = NULL;

    // Look for the previous adjacent free block
    while (iter < (char *)target) 
    {
        size_t currentHeader = *((size_t *)iter);
        size_t currentSize = currentHeader & ~(size_t)1;
        if (iter + currentSize == (char *)target) 
        {
            previous = (size_t *)iter;  // Find previous block
            break;
        }
        iter += currentSize;
    }
    
    // Merge with the previous block if it is free
    if (previous != NULL && ((*previous) & 1) == 0) 
    {
        size_t prev_size = *previous & ~(size_t)1;
        *previous = prev_size + chunkSize;  // Merges blocks
    }
}
