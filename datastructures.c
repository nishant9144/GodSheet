#include <stdio.h>
#include <stdlib.h>
#include "datastructures.h"

// static int default_ptr_cmp(const void *a, const void *b)
// {
//     if (a == b)
//         return 0;
//     return (a < b) ? -1 : 1;
// }

// Helper: return maximum of two integers.
static int max(int a, int b)
{
    return (a > b) ? a : b;
}

// Get the height of a node (0 if node is NULL)
static int height(AVLNode *node)
{
    return node ? node->height : 0;
}

// Create a new AVL node with the given data pointer.
static AVLNode *create_node(void *data)
{
    AVLNode *node = malloc(sizeof(AVLNode));
    if (!node)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1; // New node is a leaf.
    return node;
}

// Right rotation for AVL balancing.
static AVLNode *right_rotate(AVLNode *y)
{
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;

    // Rotate
    x->right = y;
    y->left = T2;

    // Update heights.
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x; // New root.
}

// Left rotation for AVL balancing.
static AVLNode *left_rotate(AVLNode *x)
{
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;

    // Rotate.
    y->left = x;
    x->right = T2;

    // Update heights.
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y; // New root.
}

// Get the balance factor of a node.
static int get_balance(AVLNode *node)
{
    return node ? height(node->left) - height(node->right) : 0;
}

// Recursive insertion into the AVL tree.
// If the data already exists (cmp returns 0), it is not inserted.
static AVLNode *avl_insert(AVLNode *node, void *data, int (*cmp)(const void *, const void *))
{
    if (node == NULL)
        return create_node(data);

    int comparison = cmp(data, node->data);
    if (comparison < 0)
        node->left = avl_insert(node->left, data, cmp);
    else if (comparison > 0)
        node->right = avl_insert(node->right, data, cmp);
    else
        // Duplicate data: do not insert.
        return node;

    // Update the height of the ancestor node.
    node->height = 1 + max(height(node->left), height(node->right));

    // Check balance factor.
    int balance = get_balance(node);

    // Left Left Case.
    if (balance > 1 && cmp(data, node->left->data) < 0)
        return right_rotate(node);

    // Right Right Case.
    if (balance < -1 && cmp(data, node->right->data) > 0)
        return left_rotate(node);

    // Left Right Case.
    if (balance > 1 && cmp(data, node->left->data) > 0)
    {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // Right Left Case.
    if (balance < -1 && cmp(data, node->right->data) < 0)
    {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

// Find the node with the minimum value (used in deletion).
static AVLNode *min_value_node(AVLNode *node)
{
    AVLNode *current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

// Recursive deletion from the AVL tree.
static AVLNode *avl_delete(AVLNode *root, void *data, int (*cmp)(const void *, const void *))
{
    if (root == NULL)
        return root;

    int comparison = cmp(data, root->data);
    if (comparison < 0)
        root->left = avl_delete(root->left, data, cmp);
    else if (comparison > 0)
        root->right = avl_delete(root->right, data, cmp);
    else
    {
        // Node to be deleted found.
        if ((root->left == NULL) || (root->right == NULL))
        {
            AVLNode *temp = root->left ? root->left : root->right;
            if (temp == NULL)
            {
                // No child.
                temp = root;
                root = NULL;
            }
            else
            {
                // One child: copy the child.
                *root = *temp;
            }
            free(temp);
        }
        else
        {
            // Node with two children: get the inorder successor.
            AVLNode *temp = min_value_node(root->right);
            // Copy the inorder successor's data (shallow copy).
            root->data = temp->data;
            // Delete the inorder successor.
            root->right = avl_delete(root->right, temp->data, cmp);
        }
    }

    if (root == NULL)
        return root;

    // Update height.
    root->height = 1 + max(height(root->left), height(root->right));

    int balance = get_balance(root);

    // Balance the tree.
    if (balance > 1 && get_balance(root->left) >= 0)
        return right_rotate(root);

    if (balance > 1 && get_balance(root->left) < 0)
    {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    if (balance < -1 && get_balance(root->right) <= 0)
        return left_rotate(root);

    if (balance < -1 && get_balance(root->right) > 0)
    {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

// Recursive search in the AVL tree.
static bool avl_search(AVLNode *root, void *data, int (*cmp)(const void *, const void *))
{
    if (root == NULL)
        return false;
    int comparison = cmp(data, root->data);
    if (comparison == 0)
        return true;
    else if (comparison < 0)
        return avl_search(root->left, data, cmp);
    else
        return avl_search(root->right, data, cmp);
}

// In-order traversal to print the data in sorted order.
// The print_data function is provided by the user.
static void inorder_traversal(AVLNode *root, void (*print_data)(void *))
{
    if (root == NULL)
        return;
    inorder_traversal(root->left, print_data);
    print_data(root->data);
    printf(" ");
    inorder_traversal(root->right, print_data);
}

// Recursively free the AVL tree.
// If free_data is not NULL, it is used to free the data stored in each node.
static void free_tree(AVLNode *root, void (*free_data)(void *))
{
    if (root == NULL)
        return;
    free_tree(root->left, free_data);
    free_tree(root->right, free_data);
    if (free_data)
        free_data(root->data);
    free(root);
}

/* ===== Set Interface Functions ===== */

// Create a new set with the specified comparator.
Set *create_set(int (*cmp)(const void *, const void *))
{
    Set *set = malloc(sizeof(Set));
    if (!set)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    set->root = NULL;
    set->count = 0;
    set->cmp = cmp;
    return set;
}

// Insert data into the set.
void set_insert(Set *set, void *data)
{
    if (!set_search(set, data))
    {
        set->root = avl_insert(set->root, data, set->cmp);
        set->count++; // Increase count on successful insertion.
    }
}

// Delete data from the set.
void set_delete(Set *set, void *data)
{
    // Check if the element exists before deletion.
    if (set_search(set, data))
    {
        set->root = avl_delete(set->root, data, set->cmp);
        set->count--; // Decrease count on successful deletion.
    }
}

// Search for data in the set; returns true if found.
bool set_search(Set *set, void *data)
{
    return avl_search(set->root, data, set->cmp);
}

// Print the set in sorted order.
void set_inorder(Set *set, void (*print_data)(void *))
{
    inorder_traversal(set->root, print_data);
    printf("\n");
}

// Free the set; if free_data is provided, it is used to free each element.
void free_set(Set *set, void (*free_data)(void *))
{
    free_tree(set->root, free_data);
    free(set);
}

/*
 * Test main function.
 * Define DATSTRUCTURES_TEST_MAIN when compiling this file to run the tests.
 */
#ifdef DATSTRUCTURES_TEST_MAIN
int main(void)
{
    AVLNode *root = NULL;
    int keys[] = {20, 4, 15, 70, 50, 100, 80};
    int n = sizeof(keys) / sizeof(keys[0]);

    // Insert keys.
    for (int i = 0; i < n; i++)
    {
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
