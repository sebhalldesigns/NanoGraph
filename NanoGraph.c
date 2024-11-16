


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
#include <math.h>

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

// Initialize the up and down stacks
void InitializeStacks(size_t initialCapacity);
// Free the stacks when no longer needed
void FreeStacks();

// Push a node onto the down stack
void DownStack_Push(nGraphNode_h node);

// Pop a node from the down stack
nGraphNode_h DownStack_Pop();
// Check if the down stack is empty
int DownStack_IsEmpty();

// Push a node onto the up stack
void UpStack_Push(nGraphNode_h node);

// Pop a node from the up stack
nGraphNode_h UpStack_Pop();
// Check if the up stack is empty
int UpStack_IsEmpty();

/******************************************************************************
 * MARK: LOCAL VARIABLES
 *****************************************************************************/

static nGraphNode_h *downStack = NULL;
static size_t downStackSize = 0;
static size_t downStatckCapacity = 0;

static nGraphNode_h *upStack = NULL;
static size_t upStackSize = 0;
static size_t upStatckCapacity = 0;

static size_t nodeQty = 0;


/******************************************************************************
 * MARK: GLOBAL FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

nGraphNode_h NanoGraph_CreateRootNode()
{
    nGraphNode_h node = (nGraphNode_h)malloc(sizeof(nGraphNode));
    memset(node, 0, sizeof(nGraphNode));

    printf("Created root node with ID %lu\n", node);

     nodeQty++;
    FreeStacks();
    InitializeStacks(nodeQty);

    return node;
}

nGraphNode_h NanoGraph_InsertNode(nGraphNode_h parent)
{
    if (parent == NULL) return NULL;

    nGraphNode_h node = (nGraphNode_h)malloc(sizeof(nGraphNode));
    memset(node, 0, sizeof(nGraphNode));

    //printf("Created child node with ID %lu\n", node);

    // Add child to parent
    if (parent->children == NULL) {
        parent->children = (nGraphNode_h*)malloc(sizeof(nGraphNode_h));
    } else {
        parent->children = (nGraphNode_h*)realloc(parent->children, (parent->child_count + 1) * sizeof(nGraphNode_h));
    }

    node->parent = parent;

    parent->children[parent->child_count++] = node;

    nodeQty++;
    FreeStacks();
    InitializeStacks(nodeQty);

    return node;
}

void NanoGraph_Recalculate(nGraphNode_h root) {
    if (root == NULL) return;

    // Use the down stack to traverse the tree for measurement
    DownStack_Push(root);

    // Temporary stack to reverse the order
    while (!DownStack_IsEmpty()) {
        nGraphNode_h node = DownStack_Pop();
        UpStack_Push(node);

        // Push children onto the down stack
        for (size_t i = node->child_count; i > 0; --i) {
            DownStack_Push(node->children[i - 1]);
        }
    }

    // Process nodes in reverse order for measurement
    while (!UpStack_IsEmpty()) {
        nGraphNode_h node = UpStack_Pop();
        MeasureNode(node);
    }

    // Push root node to down stack for layout
    DownStack_Push(root);

    // Traverse down the tree to layout nodes
    while (!DownStack_IsEmpty()) {
        nGraphNode_h node = DownStack_Pop();
        LayoutNode(node);

        // Push children onto the down stack in reverse order
        for (size_t i = node->child_count; i > 0; --i) {
            DownStack_Push(node->children[i - 1]);
        }
    }
}

nGraphNode_h NanoGraph_GetNextNode(nGraphNode_h node)
{
    if (node == NULL) return NULL;

    if (node->child_count > 0) {
        return node->children[0];
    }

    nGraphNode_h parent = node->parent;
    if (parent == NULL) return NULL;

    size_t index = 0;
    for (size_t i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == node) {
            index = i;
            break;
        }
    }

    if (index + 1 < parent->child_count) {
        return parent->children[index + 1];
    }

    return NULL;
}


/******************************************************************************
 * MARK: LOCAL FUNCTION IMPLEMENTATIONS
 *****************************************************************************/


// Initialize the up and down stacks
void InitializeStacks(size_t initialCapacity) {
    downStatckCapacity = initialCapacity;
    downStackSize = 0;
    downStack = (nGraphNode_h *)malloc(downStatckCapacity * sizeof(nGraphNode_h));

    upStatckCapacity = initialCapacity;
    upStackSize = 0;
    upStack = (nGraphNode_h *)malloc(upStatckCapacity * sizeof(nGraphNode_h));
}

// Free the stacks when no longer needed
void FreeStacks() {
    free(downStack);
    free(upStack);
}

// Push a node onto the down stack
void DownStack_Push(nGraphNode_h node) {
    if (downStackSize < downStatckCapacity) {
        downStack[downStackSize++] = node;
    } else {
        // Handle stack overflow (log an error or take appropriate action)
        fprintf(stderr, "Down stack overflow\n");
    }
}

// Pop a node from the down stack
nGraphNode_h DownStack_Pop() {
    if (downStackSize == 0) return NULL;
    return downStack[--downStackSize];
}

// Check if the down stack is empty
int DownStack_IsEmpty() {
    return downStackSize == 0;
}

// Push a node onto the up stack
void UpStack_Push(nGraphNode_h node) {
    if (upStackSize < upStatckCapacity) {
        upStack[upStackSize++] = node;
    } else {
        // Handle stack overflow (log an error or take appropriate action)
        fprintf(stderr, "Up stack overflow\n");
    }
}

// Pop a node from the up stack
nGraphNode_h UpStack_Pop() {
    if (upStackSize == 0) return NULL;
    return upStack[--upStackSize];
}

// Check if the up stack is empty
int UpStack_IsEmpty() {
    return upStackSize == 0;
}

void MeasureNode(nGraphNode_h node)
{
    switch (node->parentLayout) 
    {
        case LAYOUT_STACK:
        {
            switch (node->parentStackOrientation) 
            {
                case STACK_HORIZONTAL:
                {
                    node->calculatedSize.width = 0;
                    node->calculatedSize.height = node->userRect.height;
                    for (size_t i = 0; i < node->child_count; i++) {
                        node->calculatedSize.width += node->children[i]->calculatedSize.width;
                    }
                } break;
                case STACK_VERTICAL:
                {
                    node->calculatedSize.width = node->userRect.width;
                    node->calculatedSize.height = 0;
                    for (size_t i = 0; i < node->child_count; i++) {
                        node->calculatedSize.height += node->children[i]->calculatedSize.height;
                    }
                } break;
            }

        } break;
        
            
        case LAYOUT_DOCK:
        {
            node->calculatedSize.width = node->userRect.width;
            node->calculatedSize.height = node->userRect.height;
        } break;
            
        case LAYOUT_GRID:
        {
            node->calculatedSize.width = node->userRect.width;
            node->calculatedSize.height = node->userRect.height;
        } break;
            
        case LAYOUT_NONE:
        {
            node->calculatedSize.width = node->userRect.width;
            node->calculatedSize.height = node->userRect.height;   
        } break; 
            
    }
}

void LayoutNode(nGraphNode_h node)
{
    // Start with the userRect as the base rectangle
    //node->calculatedRect = node->userRect;

    switch (node->parentLayout) 
    {
        case LAYOUT_STACK: 
        {
            // Variables to track the starting positions
            float currentX = node->calculatedRect.x;
            float currentY = node->calculatedRect.y;

            switch (node->parentStackOrientation) 
            {
                case STACK_HORIZONTAL:
                {
                    for (size_t i = 0; i < node->child_count; i++) {
                        nGraphNode_h child = node->children[i];
                        child->calculatedRect.x = currentX;
                        child->calculatedRect.width = child->calculatedSize.width;
                        currentX += child->calculatedRect.width;
                        switch (child->childVerticalAlignment)
                        {
                            case VERTICAL_ALIGNMENT_TOP:
                            {
                                child->calculatedRect.y = node->calculatedRect.y;
                                child->calculatedRect.height = child->calculatedSize.height;
                            } break;
                            case VERTICAL_ALIGNMENT_CENTER:
                            {
                                child->calculatedRect.y = node->calculatedRect.y + (node->calculatedRect.height - child->calculatedSize.height) / 2;
                                child->calculatedRect.height = child->calculatedSize.height;
                            } break;
                            case VERTICAL_ALIGNMENT_BOTTOM:
                            {
                                child->calculatedRect.y = node->calculatedRect.y + node->calculatedRect.height - child->calculatedSize.height;
                                child->calculatedRect.height = child->calculatedSize.height;
                            } break;
                            default:
                            {
                                child->calculatedRect.y = node->calculatedRect.y;
                                child->calculatedRect.height = node->calculatedRect.height;
                            } break;
                        }
                    }
                } break;
                case STACK_VERTICAL:
                {
                    for (size_t i = 0; i < node->child_count; i++) {
                        nGraphNode_h child = node->children[i];
                        child->calculatedRect.y = currentY;
                        child->calculatedRect.height = child->calculatedSize.height;
                        currentY += child->calculatedRect.height;

                        switch (child->childHorizontalAlignment)
                        {
                            case HORIZONTAL_ALIGNMENT_LEFT:
                            {
                                child->calculatedRect.x = node->calculatedRect.x;
                                child->calculatedRect.width = child->calculatedSize.width;
                            } break;
                            case HORIZONTAL_ALIGNMENT_CENTER:
                            {
                                child->calculatedRect.x = node->calculatedRect.x + (node->calculatedRect.width - child->calculatedSize.width) / 2;
                                child->calculatedRect.width = child->calculatedSize.width;
                            } break;
                            case HORIZONTAL_ALIGNMENT_RIGHT:
                            {
                                child->calculatedRect.x = node->calculatedRect.x + node->calculatedRect.width - child->calculatedSize.width;
                                child->calculatedRect.width = child->calculatedSize.width;
                            } break;
                            default:
                            {
                                child->calculatedRect.x = node->calculatedRect.x;
                                child->calculatedRect.width = node->calculatedRect.width;
                            } break;
                        }
                    }
                } break;
            }
           
        } break;

        case LAYOUT_DOCK: 
        {
            // Simplified DockPanel logic, where children are docked to edges
            float left = node->calculatedRect.x;
            float top = node->calculatedRect.y;
            float right = node->calculatedRect.x + node->calculatedRect.width;
            float bottom = node->calculatedRect.y + node->calculatedRect.height;

            for (size_t i = 0; i < node->child_count; i++) 
            {
                nGraphNode_h child = node->children[i];
                if (i < node->child_count - 1) 
                {
                    /* not last child */
                    switch (child->childDockPosition) 
                    {
                        case DOCK_LEFT:
                        {
                            // assign calculated rect based on dock position
                            child->calculatedRect.x = left;
                            child->calculatedRect.width = child->calculatedSize.width;  
                            left += child->calculatedRect.width;

                            // assign vertical properties based on alignment
                            switch (child->childVerticalAlignment) 
                            {
                                case VERTICAL_ALIGNMENT_TOP:
                                {
                                    child->calculatedRect.y = top;
                                    child->calculatedRect.height = child->calculatedSize.height;
                                } break;
                                case VERTICAL_ALIGNMENT_CENTER:
                                {
                                    child->calculatedRect.y = top + (bottom - top - child->calculatedSize.height) / 2;
                                    child->calculatedRect.height = child->calculatedSize.height;
                                } break;
                                case VERTICAL_ALIGNMENT_BOTTOM:
                                {
                                    child->calculatedRect.y = bottom - child->calculatedSize.height;
                                    child->calculatedRect.height = child->calculatedSize.height;
                                } break;
                                default:
                                {
                                    child->calculatedRect.y = top;
                                    child->calculatedRect.height = bottom - top;
                                } break;
                            }
                       
                        } break;

                        case DOCK_TOP:
                        {
                            child->calculatedRect.y = top;
                            child->calculatedRect.height = child->calculatedSize.height;
                            top += child->calculatedRect.height;

                            switch (child->childHorizontalAlignment)
                            {
                                case HORIZONTAL_ALIGNMENT_LEFT:
                                {
                                    child->calculatedRect.x = left;
                                    child->calculatedRect.width = child->calculatedSize.width;
                                } break;
                                case HORIZONTAL_ALIGNMENT_CENTER:
                                {
                                    child->calculatedRect.x = left + (right - left - child->calculatedSize.width) / 2;
                                    child->calculatedRect.width = child->calculatedSize.width;
                                } break;
                                case HORIZONTAL_ALIGNMENT_RIGHT:
                                {
                                    child->calculatedRect.x = right - child->calculatedSize.width;
                                    child->calculatedRect.width = child->calculatedSize.width;
                                } break;
                                default:
                                {
                                    child->calculatedRect.x = left;
                                    child->calculatedRect.width = right - left;
                                } break;
                            }
                        } break;

                        case DOCK_RIGHT:
                        {
                            child->calculatedRect.x = right - child->calculatedRect.width;
                            child->calculatedRect.width = child->calculatedSize.width;  
                            right -= child->calculatedRect.width;

                            switch (child->childVerticalAlignment) {
                                case VERTICAL_ALIGNMENT_TOP:
                                {
                                    child->calculatedRect.y = top;
                                    child->calculatedRect.height = child->calculatedSize.height;
                                } break;
                                case VERTICAL_ALIGNMENT_CENTER:
                                {
                                    child->calculatedRect.y = top + (bottom - top - child->calculatedSize.height) / 2;
                                    child->calculatedRect.height = child->calculatedSize.height;
                                } break;
                                case VERTICAL_ALIGNMENT_BOTTOM:
                                {
                                    child->calculatedRect.y = bottom - child->calculatedSize.height;
                                    child->calculatedRect.height = child->calculatedSize.height;
                                } break;
                                default:
                                {
                                    child->calculatedRect.y = top;
                                    child->calculatedRect.height = bottom - top;
                                } break;
                            }
                        } break;

                        case DOCK_BOTTOM:
                        {
                            child->calculatedRect.y = bottom - child->calculatedRect.height;
                            child->calculatedRect.height = child->calculatedSize.height;
                            bottom -= child->calculatedRect.height;

                            switch (child->childHorizontalAlignment)
                            {
                                case HORIZONTAL_ALIGNMENT_LEFT:
                                {
                                    child->calculatedRect.x = left;
                                    child->calculatedRect.width = child->calculatedSize.width;
                                } break;
                                case HORIZONTAL_ALIGNMENT_CENTER:
                                {
                                    child->calculatedRect.x = left + (right - left - child->calculatedSize.width) / 2;
                                    child->calculatedRect.width = child->calculatedSize.width;
                                } break;
                                case HORIZONTAL_ALIGNMENT_RIGHT:
                                {
                                    child->calculatedRect.x = right - child->calculatedSize.width;
                                    child->calculatedRect.width = child->calculatedSize.width;
                                } break;
                                default:
                                {
                                    child->calculatedRect.x = left;
                                    child->calculatedRect.width = right - left;
                                } break;
                            }
                        } break;
                    }      
                        
                }
                else 
                {
                    /* last child */

                    switch (child->childHorizontalAlignment)
                    {
                        case HORIZONTAL_ALIGNMENT_LEFT:
                        {
                            child->calculatedRect.x = left;
                            child->calculatedRect.width = right - left;
                        } break;
                        case HORIZONTAL_ALIGNMENT_CENTER:
                        {
                            child->calculatedRect.x = left + (right - left - child->calculatedSize.width) / 2;
                            child->calculatedRect.width = child->calculatedSize.width;
                        } break;
                        case HORIZONTAL_ALIGNMENT_RIGHT:
                        {
                            child->calculatedRect.x = right - child->calculatedSize.width;
                            child->calculatedRect.width = child->calculatedSize.width;
                        } break;
                        default:
                        {
                            child->calculatedRect.x = left;
                            child->calculatedRect.width = right - left;
                        } break;
                    }

                    switch (child->childVerticalAlignment)
                    {
                        case VERTICAL_ALIGNMENT_TOP:
                        {
                            child->calculatedRect.y = top;
                            child->calculatedRect.height = bottom - top;
                        } break;
                        case VERTICAL_ALIGNMENT_CENTER:
                        {
                            child->calculatedRect.y = top + (bottom - top - child->calculatedSize.height) / 2;
                            child->calculatedRect.height = child->calculatedSize.height;
                        } break;
                        case VERTICAL_ALIGNMENT_BOTTOM:
                        {
                            child->calculatedRect.y = bottom - child->calculatedSize.height;
                            child->calculatedRect.height = child->calculatedSize.height;
                        } break;
                        default:
                        {
                            child->calculatedRect.y = top;
                            child->calculatedRect.height = bottom - top;
                        } break;
                    }
                    
                }
            }

        } break;

        case LAYOUT_GRID: {
            // Basic grid logic, assuming the grid is pre-defined with rows and columns
            int rows = node->parentGridProperties.rows;
            int cols = node->parentGridProperties.columns;

            float cellWidth = node->calculatedRect.width / cols;
            float cellHeight = node->calculatedRect.height / rows;

            for (size_t i = 0; i < node->child_count; i++) {
                nGraphNode_h child = node->children[i];
                int row = i / cols;
                int col = i % cols;

                child->calculatedRect.x = node->calculatedRect.x + col * cellWidth;
                child->calculatedRect.y = node->calculatedRect.y + row * cellHeight;
                child->calculatedRect.width = cellWidth;
                child->calculatedRect.height = cellHeight;

                // Recur to layout the child
            }
            break;
        }

        case LAYOUT_NONE: {
            // Use the userRect directly for positioning each child
            for (size_t i = 0; i < node->child_count; i++) {
                nGraphNode_h child = node->children[i];
                child->calculatedRect = child->userRect;

            }
            break;
        }
    }

    // Output the calculated position and size for debugging
    printf("Laid out node with ID %lu at (%f, %f, %f, %f)\n",
           node,
           node->calculatedRect.x, node->calculatedRect.y,
           node->calculatedRect.width, node->calculatedRect.height);
}