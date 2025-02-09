#include "datastructures.h"
#include <stdlib.h>

// Core AVL Operations

static int height(AVLNode* node) {
    return node ? node->height : 0;
}

static void update_height(AVLNode* node) {
    node->height = 1 + (height(node->left) > height(node->right) 
                       ? height(node->left) 
                       : height(node->right));
}

static AVLNode* rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

static AVLNode* rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

static AVLNode* balance(AVLNode* node) {
    int bf = height(node->left) - height(node->right);
    
    if (bf > 1) {
        if (height(node->left->left) < height(node->left->right))
            node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (bf < -1) {
        if (height(node->right->left) > height(node->right->right))
            node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

// Set Implementation

CellRefSet* cellref_set_create(CellRefCompare cmp) {
    CellRefSet* set = malloc(sizeof(CellRefSet));
    set->root = NULL;
    set->cmp = cmp;
    return set;
}

static AVLNode* insert_node(AVLNode* node, CellRef key, CellRefCompare cmp) {
    if (!node) {
        AVLNode* new_node = malloc(sizeof(AVLNode));
        new_node->key = key;
        new_node->left = new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    int comparison = cmp(&key, &node->key);
    if (comparison < 0)
        node->left = insert_node(node->left, key, cmp);
    else if (comparison > 0)
        node->right = insert_node(node->right, key, cmp);
    
    update_height(node);
    return balance(node);
}

void cellref_set_insert(CellRefSet* set, CellRef key) {
    set->root = insert_node(set->root, key, set->cmp);
}

static bool contains_node(AVLNode* node, CellRef key, CellRefCompare cmp) {
    if (!node) return false;
    
    int comparison = cmp(&key, &node->key);
    if (comparison < 0) return contains_node(node->left, key, cmp);
    if (comparison > 0) return contains_node(node->right, key, cmp);
    return true;
}

bool cellref_set_contains(CellRefSet* set, CellRef key) {
    return contains_node(set->root, key, set->cmp);
}

// Iterator Implementation

CellRefIterator* cellref_iterator(CellRefSet* set) {
    CellRefIterator* it = malloc(sizeof(CellRefIterator));
    it->capacity = 16;
    it->top = 0;
    it->stack = malloc(it->capacity * sizeof(AVLNode*));
    
    AVLNode* curr = set->root;
    while (curr) {
        if (it->top >= it->capacity) {
            it->capacity *= 2;
            it->stack = realloc(it->stack, it->capacity * sizeof(AVLNode*));
        }
        it->stack[it->top++] = curr;
        curr = curr->left;
    }
    return it;
}

bool cellref_iterator_has_next(CellRefIterator* it) {
    return it->top > 0;
}

CellRef cellref_iterator_next(CellRefIterator* it) {
    AVLNode* node = it->stack[--it->top];
    CellRef result = node->key;
    
    AVLNode* curr = node->right;
    while (curr) {
        if (it->top >= it->capacity) {
            it->capacity *= 2;
            it->stack = realloc(it->stack, it->capacity * sizeof(AVLNode*));
        }
        it->stack[it->top++] = curr;
        curr = curr->left;
    }
    return result;
}

void cellref_iterator_free(CellRefIterator* it) {
    free(it->stack);
    free(it);
}

// ====================
// Cleanup Operations
// ====================

static void free_tree(AVLNode* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

void cellref_set_destroy(CellRefSet* set) {
    free_tree(set->root);
    free(set);
}