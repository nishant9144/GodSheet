#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "spreadsheet.h"

// typedef struct AVLNode {
//     Cell *cell;
//     struct AVLNode *left;
//     struct AVLNode *right;
//     int height;
// } AVLNode;

// AVLNode* avlInsert(AVLNode* node, Cell *cell);
// AVLNode* avlDelete(AVLNode* root, Cell *cell);
// int avlSearch(AVLNode* root, Cell *cell);
// void avlInorder(AVLNode* root);
// void freeAVL(AVLNode* root);

// #endif 
typedef struct Set Set;

// Comparison function for elements.
// Should return a negative value if a < b, zero if a == b,
// and a positive value if a > b.
typedef int (*set_compare_fn)(const void *a, const void *b);

// Function for printing an element.
typedef void (*set_print_fn)(void *data);

// Function for freeing an element.
typedef void (*set_free_fn)(void *data);

/* ============================================================
   Set Interface Functions
   ============================================================ */

/**
 * @brief Creates a new set using the provided comparator.
 *
 * @param cmp A pointer to a comparison function for your data.
 * @return A pointer to the newly created set.
 */
Set *create_set(set_compare_fn cmp);

/**
 * @brief Inserts an element into the set.
 *
 * If an element considered equal (by the comparator) already exists,
 * the insertion is ignored.
 *
 * @param set Pointer to the set.
 * @param data Pointer to the data element to insert.
 */
void set_insert(Set *set, void *data);

/**
 * @brief Deletes an element from the set.
 *
 * @param set Pointer to the set.
 * @param data Pointer to the data element to delete.
 */
void set_delete(Set *set, void *data);

/**
 * @brief Searches for an element in the set.
 *
 * @param set Pointer to the set.
 * @param data Pointer to the data element to search for.
 * @return true if the element is found; false otherwise.
 */
bool set_search(Set *set, void *data);

/**
 * @brief Performs an in-order traversal of the set.
 *
 * Elements are processed in sorted order. The provided print function
 * is called for each element.
 *
 * @param set Pointer to the set.
 * @param print_data Function pointer used to print each element.
 */
void set_inorder(Set *set, set_print_fn print_data);

/**
 * @brief Frees the set and all of its nodes.
 *
 * If free_data is not NULL, it is called on each element before the
 * node is freed.
 *
 * @param set Pointer to the set.
 * @param free_data Function pointer used to free each element (or NULL).
 */
void free_set(Set *set, set_free_fn free_data);
#endif 
