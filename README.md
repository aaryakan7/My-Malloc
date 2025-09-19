**Authors:**
Aarya Kansara, Subham Jena

**Memory Management:**

Our custom allocator uses a fixed-size (4096-byte) global memory pool declared as a union to enforce 8-byte alignment.
The heap is modeled as a sequence of variable-sized chunks. Each chunk starts with an 8-byte header stored as a size_t.
In our implementation, the header packs the total size of the chunk (header plus payload) and an allocation flag in the least-significant bit.
When a request is made to allocate memory, the requested size is first rounded up to a multiple of 8. If a free chunk is larger than needed, it is split; otherwise, the entire chunk is allocated.
The free() function marks the chunk as free and automatically coalesces adjacent free chunks to reduce fragmentation.
A leak detector is registered via atexit() during initialization. It scans the entire heap upon program termination and reports any memory that was allocated but not freed.

**Testing and Stress:**

The test programs (memtest.c and memgrind.c) verify both correctness (data integrity and error detection) and performance (stress testing a large number of memory operations).
Macros in mymalloc.h automatically replace calls to malloc() and free() with our custom versions (mymalloc() and myfree()), passing FILE and LINE for better error reporting.

**Correctness Tests:**

We used the pre-developed test program (memtest.c) given to us that allocates memory using our custom malloc() and then frees it.
The program fills each allocated object with a distinct byte pattern and then verifies that no data corruption has occurred.
It also checks for proper error detection (e.g., freeing a non-allocated pointer or double-free).
When memtest.c runs correctly, it reports “0 incorrect bytes.”
As per assignment instructions, (memtest.c) was not included in the final .tar file.

**Stress Testing:**

The program memgrind.c performs a workload of five tasks:
- Test 1: Calls malloc() and immediately free() a 1-byte object 120 times.
  
- Test 2: Allocates 120 1-byte objects (storing pointers in an array) and then frees them.
  
- Test 3: Creates an array of 120 pointers; repeatedly makes a random choice between allocating a 1-byte object (if a slot is free) or freeing an allocated object. After 120 allocations have been performed, any remaining objects are freed.
  
- Test 4: Builds a singly linked list of 50 nodes, traverses it (simulating processing), and then frees all nodes.
  
- Test 5: Constructs a binary search tree (BST) by inserting 50 random keys, performs an in-order traversal (to simulate processing), and then frees the entire tree.

memgrind.c runs these five tests sequentially 50 times, using gettimeofday() to record the overall start and end times.
It then computes and displays the average time per workload iteration.

**Execution:**

To verify correctness, you can run (memgrind.c) which is provided.
Use the provided Makefile to compile the project by running this command in the terminal: make
To run the stress test program, execute this command in the terminal: ./memgrind
