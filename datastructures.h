#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "spreadsheet.h"
#include <stdbool.h>

// Cell reference comparator type
typedef int (*CellRefCompare)(const CellRef*, const CellRef*);

// AVL Node specialized for CellRef
typedef struct AVLNode{
    CellRef key;
   struct AVLNode *left;
   struct AVLNode *right;
   int height;
} AVLNode;

// CellRef Set structure
typedef struct {
    AVLNode* root;
    CellRefCompare cmp;
} CellRefSet;

// Iterator structure for CellRefSet
typedef struct {
    AVLNode** stack;
    int top;
    int capacity;
} CellRefIterator;

// Function prototypes
CellRefSet *cellref_set_create(int (*cmp)(const CellRef *, const CellRef *));
void cellref_set_insert(CellRefSet *set, CellRef key);
bool cellref_set_contains(CellRefSet *set, CellRef key);
void cellref_set_remove(CellRefSet *set, CellRef key);
void cellref_set_destroy(CellRefSet *set);


// Iterator Interface
CellRefIterator* cellref_iterator(CellRefSet* set);
bool cellref_iterator_has_next(CellRefIterator* it);
CellRef cellref_iterator_next(CellRefIterator* it);
void cellref_iterator_free(CellRefIterator* it);

#endif
