#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"

// malloc() and immediately free() a 1-byte object, 120 times
void test1(void) 
{
    for (int i = 0; i < 120; i++) 
    {
        char *ptr = malloc(1);
        free(ptr);
    }
}

// Use malloc() to get 120 1-byte objects, storing the pointers in an array, then use free() to deallocate the chunks
void test2(void) 
{
    char *ptrArray[120];

    for (int i = 0; i < 120; i++) 
    {
        ptrArray[i] = malloc(1);
    }

    for (int i = 0; i < 120; i++) 
    {
        free(ptrArray[i]);
    }
}

// Create an array of 120 pointers. Repeatedly make a random choice between:
    // allocating a 1-byte object and adding the pointer to the array
    // deallocating a previously allocated object
// Once 120 allocations have been performed, deallocate all objects
void test3(void)
{
    char *ptrArray[120];

    // Initalize all pointers to null
    for (int i = 0; i < 120; i++) 
    {
        ptrArray[i] = NULL;
    }

    int count = 0; 
    int allocations = 0;
        
    // Keep looping till 120 allocations are performed
    while (allocations < 120) 
    {
        if (count == 0 || (rand() % 2 == 0 && count < 120)) 
        {
            for (int j = 0; j < 120; j++) 
            {
                // Allocate memory to the first null pointer
                if (ptrArray[j] == NULL) 
                {
                    ptrArray[j] = malloc(1);
                    count++;
                    allocations++;
                    break;
                }
            }
        } 
            
        else 
        {
            // Free memory from the first allocated block that is found
            for (int j = 0; j < 120; j++) 
            {
                if (ptrArray[j] != NULL) 
                {
                    free(ptrArray[j]);
                    ptrArray[j] = NULL;
                    count--;
                    break;
                }
            }
        }
    }

    // Make sure that all the allocated memory is freed
    for (int i = 0; i < 120; i++) 
    {
        if (ptrArray[i] != NULL) 
        {
            free(ptrArray[i]);
            ptrArray[i] = NULL;
        }
    }
}


void test4(void) 
{
    // Singly linked list node struct
    typedef struct node 
    {
        int data;
        struct node *next;
    } node;
    
    node *head = NULL;

    // Create a linked list with 50 nodes
    for (int i = 0; i < 50; i++) 
    {
        node *new_node = malloc(sizeof(node));
        
        new_node -> data = i;
        new_node -> next = head;
        head = new_node;
    }

    // Compute the sum of all the node values
    node *current = head;
    long sum = 0;
    
    while (current != NULL) 
    {
        sum += current -> data;
        current = current -> next;
    }
    
    // Free the allocated memory
    current = head;
    
    while (current != NULL) 
    {
        node *temp = current;
        current = current -> next;
        free(temp);
    }
}

// BST node struct
typedef struct bstNode 
{
    struct bstNode *left;
    struct bstNode *right;
    int key;
} bstNode;

// Function that inserts a key into the BST structure
bstNode* insertNodeBST(bstNode *root, int key) 
{
    if (root == NULL) 
    {
        bstNode *new_node = malloc(sizeof(bstNode));

        if (!new_node) 
        {
            exit(1);
        }

        new_node -> key = key;
        new_node -> left = new_node -> right = NULL;
        return new_node;
    }

    if (key < root -> key)
        root -> left = insertNodeBST(root -> left, key);

    else if (key > root -> key)
        root -> right = insertNodeBST(root -> right, key);
    return root;
}

// Frees memory allocated for the BST
void freeBST(bstNode *root) 
{
    if (root == NULL) return;
    freeBST(root -> left);
    freeBST(root -> right);
    free(root);
}

// Traverses the BST in order
void inorder(bstNode *root) 
{
    if (root == NULL) return;
    inorder(root -> left);
    inorder(root -> right);
}


void test5(void) 
{
    bstNode *root = NULL;
    int n = 50;

    // Inserts 50 random keys into the BST
    for (int i = 0; i < n; i++) 
    {
        int key = rand() % 1000;
        root = insertNodeBST(root, key);
    }

    inorder(root);
    freeBST(root);
}

// Function that prints out a formatted timestamp
void printTime(struct timeval tv, const char *label) 
{
    time_t sec = tv.tv_sec;
    struct tm *tm_info = localtime(&sec);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s: %s.%06ld\n", label, timeStr, tv.tv_usec);
}

int main(void) 
{
    srand((unsigned int)time(NULL));
    
    struct timeval start, end;
    
    gettimeofday(&start, NULL); // Record starting time
    
    // Executes the test functions 50 times
    for (int i = 0; i < 50; i++) 
    {
        test1();
        test2();
        test3();
        test4();
        test5();
    }
 
    gettimeofday(&end, NULL); // Record ending time
    
    // Computes the total and average execution time
    long total_time_us = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    double avg_time = total_time_us / (double)50;
    
    // Outputs the formatted times
    printTime(start, "Start time");
    printTime(end, "End time");
    printf("Average workload time: %.2f microseconds\n", avg_time);
    
    return 0;
}