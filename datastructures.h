#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "spreadsheet.h"


/* AVL Tree Node: Each node stores a pointer to a Cell. */
typedef struct AVLNode {
    Cell *cell;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
} AVLNode;

/* 
 * Inserts a cell into the AVL tree rooted at `node`.
 * Duplicate cells are ignored.
 * Returns the new root of the subtree.
 */
AVLNode* avlInsert(AVLNode* node, Cell *cell);

/* 
 * Deletes a cell from the AVL tree rooted at `root`.
 * Returns the new root of the subtree.
 */
AVLNode* avlDelete(AVLNode* root, Cell *cell);

/* 
 * Searches for a key in the AVL tree.
 * Returns 1 if found, 0 otherwise.
 */
int avlSearch(AVLNode* root, Cell *cell);

/* 
 * Performs an inorder traversal of the AVL tree.
 * Useful for debugging or displaying the set contents.
 */
void avlInorder(AVLNode* root);

/*  Frees all nodes in the AVL tree. */
void freeAVL(AVLNode* root);

#endif /* DATASTRUCTURES_H */
