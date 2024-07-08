#pragma once
#include <stddef.h>
// Define the structure for a node in the linked list
typedef struct Node Node;

Node** initializeList();
void freeHead(Node** head);
void print_struct(Node* head);
int getElement(Node* head, int index);
void insert(Node** head, int data);
void freeList(Node* head);
int pop(Node** head);

