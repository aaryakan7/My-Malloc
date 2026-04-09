#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"

// Task 1: malloc() and immediately free() a 1-byte object, 120 times
void test1(void) {
    for (int i = 0; i < 120; i++) {
        char *ptr = malloc(1);
        free(ptr);
    }
}

// Task 2: Use malloc() to get 120 1-byte objects, storing the pointers in an array, then use free()
void test2(void) {
    char *ptrArray[120];
    for (int i = 0; i < 120; i++) {
        ptrArray[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptrArray[i]);
    }
}

// Task 3: Randomly allocate or deallocate 1-byte objects until 120 allocations are performed 
void test3(void) {
    char *ptrArray[120];
    for (int i = 0; i < 120; i++) ptrArray[i] = NULL;

    int current_count = 0; 
    int total_allocations = 0;
        
    while (total_allocations < 120) {
        if (current_count == 0 || (rand() % 2 == 0 && current_count < 120)) {
            for (int j = 0; j < 120; j++) {
                if (ptrArray[j] == NULL) {
                    ptrArray[j] = malloc(1);
                    current_count++;
                    total_allocations++;
                    break;
                }
            }
        } else {
            for (int j = 0; j < 120; j++) {
                if (ptrArray[j] != NULL) {
                    free(ptrArray[j]);
                    ptrArray[j] = NULL;
                    current_count--;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < 120; i++) {
        if (ptrArray[i] != NULL) free(ptrArray[i]);
    }
}

// Task 4: Linked List manipulation (Non-trivial workload)
void test4(void) {
    typedef struct node {
        int data;
        struct node *next;
    } node;
    
    node *head = NULL;
    for (int i = 0; i < 50; i++) {
        node *new_node = malloc(sizeof(node));
        if (new_node) {
            new_node->data = i;
            new_node->next = head;
            head = new_node;
        }
    }
    
    node *current = head;
    while (current != NULL) {
        node *temp = current;
        current = current->next;
        free(temp);
    }
}

// Task 5: BST manipulation (Non-trivial workload)
typedef struct bstNode {
    struct bstNode *left, *right;
    int key;
} bstNode;

bstNode* insert(bstNode *root, int key) {
    if (root == NULL) {
        bstNode *new_node = malloc(sizeof(bstNode));
        if (new_node) {
            new_node->key = key;
            new_node->left = new_node->right = NULL;
        }
        return new_node;
    }
    if (key < root->key) root->left = insert(root->left, key);
    else if (key > root->key) root->right = insert(root->right, key);
    return root;
}

void freeBST(bstNode *root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    free(root);
}

void test5(void) {
    bstNode *root = NULL;
    for (int i = 0; i < 50; i++) {
        root = insert(root, rand() % 1000);
    }
    freeBST(root);
}

int main(void) {
    struct timeval start, end;
    srand(time(NULL));

    gettimeofday(&start, NULL); 
    
    for (int i = 0; i < 50; i++) {
        test1();
        test2();
        test3();
        test4();
        test5();
    }
 
    gettimeofday(&end, NULL); 
    
    long total_time = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    printf("Average workload time: %.2f microseconds\n", total_time / 50.0);
    
    return 0;
}
