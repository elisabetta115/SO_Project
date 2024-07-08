#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "Stack.h"
#include "Malloc.h"

#define DEBUG

typedef struct Node {
    int data;
    struct Node* next;
} Node;


// Function to initialize the stack
Stack initializeStack() {
    Stack stack = pseudo_malloc(sizeof(Node *));
    if(stack == NULL){
        return NULL;
    }
    *stack = NULL;
    return stack;
}

// Function to free the memory allocated for the linked list
int freeList(Node* head) {
    struct Node* current = head;
    struct Node* next;

    // Traverse the linked list and free each node
    while (current != NULL) {
        next = current->next;
        if(pseudo_free(current) ==-1){
            printf("Corrente: %d\n", current->data);
            return -1;
        }
        current = next;
    }
    return 1;
}

// Function to free the memory allocated for the stack
int destroyStack(Stack stack) {
    if(freeList(*stack) == -1){
        printf("Error in freeList\n");
        return -1;
    }
    if(pseudo_free(stack) == -1){
        printf("Error in stack\n");
        return -1;
    }
    return 1;
}

// Function to add a new node at the beginning of the linked list
int insert(Stack stack, int data) {
    struct Node* newNode = (struct Node*)pseudo_malloc(sizeof(struct Node));
    if(newNode == NULL){
        errno = EINVAL;
        return -1;
    }
    newNode->data = data;
    newNode->next = *stack;
    *stack = newNode;
    return 0;
}

// Function to get the element at a specific index in the linked list
int getElement(Stack stack, int index) {
    struct Node* current = *stack;
    int count = 0;
    while (current != NULL) {
        if (count == index) {
            return current->data;
        }
        current = current->next;
        count++;
    }
    return -1;
}

#ifdef DEBUG
// Function to print the elements of the linked list
void printStack(Stack stack) {
    struct Node* current = *stack;
    while (current != NULL) {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}
#endif

// Function to remove the first node from the linked list
int pop(Stack stack) {
    if (*stack == NULL) {
        return -1;
    }
    int data = (*stack)->data;
    Node* temp = *stack;
    *stack = (*stack)->next;
    if(pseudo_free(temp)==-1) {
        return -1;
    }
    return data;
}