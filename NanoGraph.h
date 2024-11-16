#ifndef NANOGRAPH_H
#define NANOGRAPH_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct nGraphNode* nGraphNode_h;

typedef enum
{
    LAYOUT_STACK,
    LAYOUT_DOCK,
    LAYOUT_GRID,
    LAYOUT_ABSOLUTE
} nGraphParentLayout;

typedef enum
{
    STACK_HORIZONTAL,
    STACK_VERTICAL
} nGraphParentStackOrientation;

typedef enum
{
    GRID_UNIT_PIXEL,
    GRID_UNIT_PERCENTAGE
} nGraphGridUnit;

typedef struct 
{
    nGraphGridUnit unit;
    float value;

    float minValue;
    float maxValue;
    
    int priority;
} nGraphGridMeasurement;

typedef struct 
{
    size_t rows;
    size_t columns;
    nGraphGridMeasurement* rowSizes;
    nGraphGridMeasurement* columnSizes;
} nGraphParentGridProperties;

typedef enum 
{
    DOCK_TOP,
    DOCK_BOTTOM,
    DOCK_LEFT,
    DOCK_RIGHT
} nGraphChildDockPosition;

typedef struct 
{
    size_t row;
    size_t column;
    size_t rowSpan;
    size_t columnSpan;
} nGraphChildGridPosition;

typedef struct
{
    float x;
    float y;
    float width;
    float height;
} nGraphRect;

typedef struct 
{
    float width;
    float height;
} nGraphSize;

typedef struct nGraphNode 
{
    nGraphParentLayout parentLayout;

    nGraphParentStackOrientation parentStackOrientation;
    nGraphParentGridProperties parentGridProperties;

    nGraphChildDockPosition childDockPosition;
    nGraphChildGridPosition childGridPosition;

    nGraphRect userRect;

    nGraphSize calculatedSize;
    nGraphRect calculatedRect;

    float red;
    float green;
    float blue;
    
    nGraphNode_h parent;
    nGraphNode_h next;
    nGraphNode_h* children;
    size_t child_count;
} nGraphNode;

nGraphNode_h NanoGraph_CreateRootNode();

nGraphNode_h NanoGraph_InsertNode(nGraphNode_h parent);

void NanoGraph_Recalculate(nGraphNode_h node);

nGraphNode_h NanoGraph_GetNextNode(nGraphNode_h node);


#endif // NANOGRAH_H