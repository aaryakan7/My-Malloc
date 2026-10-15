# My-Malloc

## Authors

Aarya Kansara and Subham Jena

## Allocator design

The allocator manages a fixed 4096-byte pool held in a static union aligned to `double`. Each block begins with an 8-byte header containing the total block size and an allocated flag in its low bit. Block sizes are multiples of 8, and the smallest block is 16 bytes: an 8-byte header and at least 8 payload bytes.

Allocation uses first fit. Requests are rounded up to an 8-byte boundary, and a free block is split only when the remainder can hold a minimum-sized block. Freeing validates that the pointer is exactly the start of an allocated payload, then joins adjacent free blocks. Requests that do not fit return `NULL` with a diagnostic. Invalid frees print a diagnostic and exit with status 2. A detector registered during initialization reports allocated payload bytes and object count at process exit.

## Build and run

Run `make` to build `memgrind` and `memcheck`. Run them with:

```sh
./memgrind
./memcheck
```

`memgrind` runs five workloads 50 times and prints the average time per workload iteration in microseconds. `memcheck` checks alignment, separate allocations, data integrity, block reuse and coalescing, pool capacity, and overflow rejection.

`memcheck` also accepts three arguments for checking invalid-free behavior. Each invocation is expected to print `free: Inappropriate pointer` and exit with status 2:

```sh
./memcheck outside-free
./memcheck interior-free
./memcheck double-free
```

## Benchmark workloads

- Task 1 allocates and immediately frees a one-byte block 120 times.
- Task 2 allocates 120 one-byte blocks, then frees them.
- Task 3 randomly allocates or frees one-byte blocks until it has made 120 allocations, then frees any blocks left allocated.
- Task 4 builds and frees a linked list of 50 nodes.
- Task 5 builds and frees a binary search tree using 50 random keys.

The benchmark uses `gettimeofday()` around all 50 rounds and reports the elapsed time divided by 50.
