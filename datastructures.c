#include <stdio.h>
#include <stdlib.h>
#include "datastructures.h"

/* Utility function: returns the maximum of two integers. */
static int max(int a, int b) {
    return (a > b) ? a : b;
}

/* Returns the height of the node (0 if node is NULL). */
static int height(AVLNode *node) {
    return (node == NULL) ? 0 : node->height;
}

/* Allocates and returns a new AVL tree node with the given cell. */
static AVLNode* newNode(Cell *cell) {
    AVLNode *node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    node->cell = cell;
    node->left = node->right = NULL;
    node->height = 1;  // New node is initially a leaf.
    return node;
}

/* Right-rotates the subtree rooted with y and returns the new root. */
static AVLNode* rightRotate(AVLNode* y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;

    // Perform rotation.
    x->right = y;
    y->left = T2;

    // Update heights.
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

/* Left-rotates the subtree rooted with x and returns the new root. */
static AVLNode* leftRotate(AVLNode* x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;

    // Perform rotation.
    y->left = x;
    x->right = T2;

    // Update heights.
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

/* Returns the balance factor of the node (difference between left and right heights). */
static int getBalance(AVLNode* N) {
    return (N == NULL) ? 0 : height(N->left) - height(N->right);
}

AVLNode* avlInsert(AVLNode* node, Cell *cell) {
    /* 1. Perform the normal BST insertion. */
    if (node == NULL)
        return newNode(cell);

    if (cell < node->cell)
        node->left  = avlInsert(node->left, cell);
    else if (cell > node->cell)
        node->right = avlInsert(node->right, cell);
    else  // Duplicate cells are not allowed in the set.
        return node;

    /* 2. Update height of the ancestor node. */
    node->height = 1 + max(height(node->left), height(node->right));

    /* 3. Get the balance factor to check whether this node became unbalanced. */
    int balance = getBalance(node);

    // Left Left Case.
    if (balance > 1 && cell < node->left->cell)
        return rightRotate(node);

    // Right Right Case.
    if (balance < -1 && cell > node->right->cell)
        return leftRotate(node);

    // Left Right Case.
    if (balance > 1 && cell > node->left->cell) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    // Right Left Case.
    if (balance < -1 && cell < node->right->cell) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    /* Return the (unchanged) node pointer. */
    return node;
}

/* Utility function: returns the node with the smallest cell value found in the tree. */
static AVLNode* minValueNode(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

AVLNode* avlDelete(AVLNode* root, Cell *cell) {
    if (root == NULL)
        return root;

    /* 1. Perform standard BST delete. */
    if (cell < root->cell)
        root->left = avlDelete(root->left, cell);
    else if (cell > root->cell)
        root->right = avlDelete(root->right, cell);
    else {
        // Node with one child or no child.
        if ((root->left == NULL) || (root->right == NULL)) {
            AVLNode *temp = root->left ? root->left : root->right;

            // No child case.
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                // One child: Copy the contents of the non-null child.
                *root = *temp;
            }
            free(temp);
        } else {
            // Node with two children: Get the inorder successor (smallest in the right subtree).
            AVLNode* temp = minValueNode(root->right);
            root->cell = temp->cell;
            root->right = avlDelete(root->right, temp->cell);
        }
    }

    // If the tree had only one node then return.
    if (root == NULL)
        return root;

    /* 2. Update the height of the current node. */
    root->height = 1 + max(height(root->left), height(root->right));

    /* 3. Check the balance factor. */
    int balance = getBalance(root);

    // Left Left Case.
    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);

    // Left Right Case.
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    // Right Right Case.
    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);

    // Right Left Case.
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

int avlSearch(AVLNode* root, Cell *cell) {
    if (root == NULL)
        return 0;
    if (cell < root->cell)
        return avlSearch(root->left, cell);
    else if (cell > root->cell)
        return avlSearch(root->right, cell);
    else
        return 1;  // Cell found.
}

void avlInorder(AVLNode* root) {
    if (root) {
        avlInorder(root->left);
        printf("%p ", root->cell);
        avlInorder(root->right);
    }
}

void freeAVL(AVLNode* root) {
    if (root) {
        freeAVL(root->left);
        freeAVL(root->right);
        free(root);
    }
}

/* 
 * Test main function.
 * Define DATSTRUCTURES_TEST_MAIN when compiling this file to run the tests.
 */
#ifdef DATSTRUCTURES_TEST_MAIN
int main(void) {
    AVLNode* root = NULL;
    int keys[] = {20, 4, 15, 70, 50, 100, 80};
    int n = sizeof(keys) / sizeof(keys[0]);

    // Insert keys.
    for (int i = 0; i < n; i++) {
        root = avlInsert(root, keys[i]);
    }

    printf("Inorder traversal of the AVL set is:\n");
    avlInorder(root);
    printf("\n");

    // Test search.
    int searchKey = 70;
    printf("Searching for %d: %s\n", searchKey,
           avlSearch(root, searchKey) ? "found" : "not found");

    // Delete a key.
    root = avlDelete(root, 70);
    printf("Inorder traversal after deleting %d:\n", searchKey);
    avlInorder(root);
    printf("\n");

    freeAVL(root);
    return 0;
}
#endif
