typedef struct Node Node;
typedef Node** Stack;

Stack initializeStack();
int destroyStack(Stack stack);
int getElement(Stack stack, int index);
int insert(Stack stack, int data);
int pop(Stack stack);

#ifdef DEBUG
void printStack(Stack stack);
#endif