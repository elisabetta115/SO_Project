#include <stdio.h>
#include <stdlib.h>

#include "Stack.h"
#include "Malloc.h"

typedef struct Node {
    int data;
    struct Node* next;
} Node;


// Function to initialize the head of the linked list
Node** initializeList() {
    Node** head = pseudo_malloc(sizeof(Node *));
    *head = NULL;
    return head;
}

// Function to free the memory allocated for the head of the linked list
void freeHead(Node** head) {
    freeList(*head);
    pseudo_free(*head);
}

// Function to add a new node at the beginning of the linked list
void insert(Node** head, int data) {
    // Create a new node
    struct Node* newNode = (struct Node*)pseudo_malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = *head;

    // Update the head pointer to point to the new node
    *head = newNode;
}

// Function to get the element at a specific index in the linked list
int getElement(Node* head, int index) {
    struct Node* current = head;
    int count = 0;

    // Traverse the linked list until the desired index is reached
    while (current != NULL) {
        if (count == index) {
            return current->data;
        }
        current = current->next;
        count++;
    }

    // If the index is out of bounds, return -1 or any other appropriate value
    return -1;
}
// Function to print the elements of the linked list
void print_struct(Node* head) {
    struct Node* current = head;
    while (current != NULL) {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}
// Function to free the memory allocated for the linked list
void freeList(Node* head) {
    struct Node* current = head;
    struct Node* next;

    // Traverse the linked list and free each node
    while (current != NULL) {
        next = current->next;
        pseudo_free(current);
        current = next;
    }
}

// Function to remove the first node from the linked list
int pop(Node** head) {
    if (*head == NULL) {
        return -1;
    }
    int data = (*head)->data;
    Node* temp = *head;
    *head = (*head)->next;
    pseudo_free(temp);
    return data;
}