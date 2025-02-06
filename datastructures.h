#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

/* AVL Tree Node: Each node stores an integer key. */
typedef struct AVLNode {
    int key;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
} AVLNode;

/* 
 * Inserts a key into the AVL tree rooted at `node`.
 * Duplicate keys are ignored.
 * Returns the new root of the subtree.
 */
AVLNode* avlInsert(AVLNode* node, int key);

/* 
 * Deletes a key from the AVL tree rooted at `root`.
 * Returns the new root of the subtree.
 */
AVLNode* avlDelete(AVLNode* root, int key);

/* 
 * Searches for a key in the AVL tree.
 * Returns 1 if found, 0 otherwise.
 */
int avlSearch(AVLNode* root, int key);

/* 
 * Performs an inorder traversal of the AVL tree.
 * Useful for debugging or displaying the set contents.
 */
void avlInorder(AVLNode* root);

/* 
 * Frees all nodes in the AVL tree.
 */
void freeAVL(AVLNode* root);

#endif /* DATASTRUCTURES_H */
