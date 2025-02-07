#include <stdlib.h>
#include <stdbool.h>
#include "sheet.h"

#define INITIAL_SET_CAPACITY 10
#define INITIAL_LIST_CAPACITY 10

// Define a hash table node to handle collisions through chaining
typedef struct HashNode {
    Cell* cell;
    struct HashNode* next;
} HashNode;

// Hash function for Cell pointers
static size_t hash_cell(Cell* cell, size_t capacity) {
    // Convert pointer to integer and use multiplicative hashing
    size_t key = (size_t)cell;
    const size_t A = 2654435769u;  // Golden ratio multiplier
    return ((key * A) >> 32) % capacity;
}

CellSet* create_cell_set(size_t initial_capacity) {
    CellSet* set = malloc(sizeof(CellSet));
    // Use power of 2 for capacity to optimize modulo operations
    set->capacity = initial_capacity > 16 ? initial_capacity : 16;
    set->size = 0;
    set->load_factor = 0.75f;
    set->buckets = calloc(set->capacity, sizeof(HashNode*));  // Initialize all buckets to NULL
    set->cells = create_cell_list();
    return set;
}

// Helper function to resize the hash table when it gets too full
static void resize_cell_set(CellSet* set) {
    size_t old_capacity = set->capacity;
    HashNode** old_buckets = set->buckets;
    
    // Double the capacity
    set->capacity *= 2;
    set->buckets = calloc(set->capacity, sizeof(HashNode*));
    set->size = 0;
    
    // Rehash all existing elements
    for (size_t i = 0; i < old_capacity; i++) {
        HashNode* current = old_buckets[i];
        while (current != NULL) {
            HashNode* next = current->next;
            // Rehash the node into the new bucket array
            size_t new_index = hash_cell(current->cell, set->capacity);
            current->next = set->buckets[new_index];
            set->buckets[new_index] = current;
            set->size++;
            current = next;
        }
    }
    
    free(old_buckets);
}

void cell_set_add(CellSet* set, Cell* cell) {
    // Check if resizing is needed
    if ((float)set->size / set->capacity >= set->load_factor) {
        resize_cell_set(set);
    }
    
    if(cell_set_contains(set, cell)){
        return;
    }

    size_t index = hash_cell(cell, set->capacity);
    
    // // Check if cell already exists
    // HashNode* current = set->buckets[index];
    // while (current != NULL) {
    //     if (current->cell == cell) {
    //         return;  // Cell already exists
    //     }
    //     current = current->next;
    // }
    
    // Create new node and add it to the front of the chain
    HashNode* new_node = malloc(sizeof(HashNode));
    new_node->cell = cell;
    new_node->next = set->buckets[index];
    set->buckets[index] = new_node;
    cell_list_add(set->cells, cell);
    set->size++;
}

// void cell_set_remove(CellSet* set, Cell* cell) {
//     size_t index = hash_cell(cell, set->capacity);
    
//     HashNode* current = set->buckets[index];
//     HashNode* prev = NULL;
    
//     while (current != NULL) {
//         if (current->cell == cell) {
//             if (prev == NULL) {
//                 // Removing first node in chain
//                 set->buckets[index] = current->next;
//             } else {
//                 // Removing from middle or end of chain
//                 prev->next = current->next;
//             }
//             free(current);
//             set->size--;
//             return;
//         }
//         prev = current;
//         current = current->next;
//     }
// }

bool cell_set_contains(CellSet* set, Cell* cell) {
    size_t index = hash_cell(cell, set->capacity);
    
    HashNode* current = set->buckets[index];
    while (current != NULL) {
        if (current->cell == cell) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void cell_set_clear(CellSet* set) {
    // Free all nodes in all buckets
    for (size_t i = 0; i < set->capacity; i++) {
        HashNode* current = set->buckets[i];
        while (current != NULL) {
            HashNode* next = current->next;
            free(current);
            current = next;
        }
        set->buckets[i] = NULL;
    }
    set->size = 0;
}

void destroy_cell_set(CellSet* set) {
    cell_set_clear(set);
    destroy_cell_list(set->cells);
    free(set->buckets);
    free(set);
}


