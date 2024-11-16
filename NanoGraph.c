


/******************************************************************************
 * NanoGraph.c
 * 
 * NanoGraph is a simple layout engine for arranging UI elements in a tree, in a
 * similar way to XAML-based systems.
 * 
 * November 2024, Seb Hall
 *****************************************************************************/

#include "NanoGraph.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STACK_BLOCK_SIZE 10

/******************************************************************************
 * MARK: TYPE DEFINITIONS
 *****************************************************************************/

typedef struct {
    nGraphNode_h* data;
    size_t size;
    size_t capacity;
} Stack;


/******************************************************************************
 * MARK: LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/

void MeasureNode(nGraphNode_h node);
void LayoutNode(nGraphNode_h node);

void Stack_Init(Stack* stack);
void Stack_Push(Stack* stack, nGraphNode_h node);
nGraphNode_h Stack_Pop(Stack* stack);
int Stack_IsEmpty(Stack* stack);
void Stack_Free(Stack* stack);

/******************************************************************************
 * MARK: LOCAL VARIABLES
 *****************************************************************************/


/******************************************************************************
 * MARK: GLOBAL FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

nGraphNode_h NanoGraph_CreateRootNode()
{
    nGraphNode_h node = (nGraphNode_h)malloc(sizeof(nGraphNode));
    memset(node, 0, sizeof(nGraphNode));

    printf("Created root node with ID %lu\n", node);

    return node;
}

nGraphNode_h NanoGraph_InsertNode(nGraphNode_h parent)
{
    if (parent == NULL) return NULL;

    nGraphNode_h node = (nGraphNode_h)malloc(sizeof(nGraphNode));
    memset(node, 0, sizeof(nGraphNode));

    printf("Created child node with ID %lu\n", node);

    // Add child to parent
    if (parent->children == NULL) {
        parent->children = (nGraphNode_h*)malloc(sizeof(nGraphNode_h));
    } else {
        parent->children = (nGraphNode_h*)realloc(parent->children, (parent->child_count + 1) * sizeof(nGraphNode_h));
    }

    parent->children[parent->child_count++] = node;

    return node;
}

void NanoGraph_Recalculate(nGraphNode_h root)
{
    if (root == NULL) return;

    Stack stack;
    Stack_Init(&stack);

    // Push root node to stack
    Stack_Push(&stack, root);

    // Temporary stack to reverse the order
    Stack tempStack;
    Stack_Init(&tempStack);

    // Traverse down the tree to measure nodes
    while (!Stack_IsEmpty(&stack)) {
        nGraphNode_h node = Stack_Pop(&stack);
        Stack_Push(&tempStack, node);

        // Push children to stack
        for (size_t i = node->child_count; i > 0; --i) {
            Stack_Push(&stack, node->children[i - 1]);
        }
    }

    // Process nodes in reverse order
    while (!Stack_IsEmpty(&tempStack)) {
        nGraphNode_h node = Stack_Pop(&tempStack);
        MeasureNode(node);
    }

    Stack_Free(&tempStack);

    // Push root node to stack again for layout
    Stack_Push(&stack, root);

    // Traverse down the tree to layout nodes
    while (!Stack_IsEmpty(&stack)) {
        nGraphNode_h node = Stack_Pop(&stack);
        LayoutNode(node);

        // Push children to stack in reverse order
        for (size_t i = node->child_count; i > 0; --i) {
            Stack_Push(&stack, node->children[i - 1]);
        }
    }

    Stack_Free(&stack);
}



/******************************************************************************
 * MARK: LOCAL FUNCTION IMPLEMENTATIONS
 *****************************************************************************/


void Stack_Init(Stack* stack) {
    stack->capacity = STACK_BLOCK_SIZE;
    stack->size = 0;
    stack->data = (nGraphNode_h*)malloc(stack->capacity * sizeof(nGraphNode_h));
}

void Stack_Push(Stack* stack, nGraphNode_h node) {
    if (stack->size == stack->capacity) {
        stack->capacity += STACK_BLOCK_SIZE;
        stack->data = (nGraphNode_h*)realloc(stack->data, stack->capacity * sizeof(nGraphNode_h));
    }
    stack->data[stack->size++] = node;
}

nGraphNode_h Stack_Pop(Stack* stack) {
    return stack->data[--stack->size];
}

int Stack_IsEmpty(Stack* stack) {
    return stack->size == 0;
}

void Stack_Free(Stack* stack) {
    free(stack->data);
}

void MeasureNode(nGraphNode_h node)
{
    // Implement measurement logic here
    printf("Measuring node with ID %lu\n", node);
}

void LayoutNode(nGraphNode_h node)
{
    // Implement layout logic here
    printf("Laying out node with ID %lu\n", node);
}