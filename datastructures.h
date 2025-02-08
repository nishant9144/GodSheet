#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "spreadsheet.h"
typedef struct AVLNode
{
   void *data;
   struct AVLNode *left;
   struct AVLNode *right;
   int height;
} AVLNode;
typedef struct Set
{
   AVLNode *root;                          // Internal AVL tree root.
   int count;                              // Number of elements in the set.
   int (*cmp)(const void *, const void *); // Comparator for elements.
} Set;

// Function prototypes for set operations:
Set *create_set(int (*cmp)(const void *, const void *));
void set_insert(Set *set, void *data);
void set_delete(Set *set, void *data);
bool set_search(Set *set, void *data);
void set_inorder(Set *set, void (*print_data)(void *));
void free_set(Set *set, void (*free_data)(void *));

#endif
