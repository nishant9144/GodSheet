#include "sheet.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_HEAP_CAPACITY 10

// Heap-based CellSet using array-backed min-heap
typedef struct {
    Cell** heap;       // Array of Cell pointers
    size_t size;       // Number of elements in heap
    size_t capacity;   // Max capacity before resizing
} CellHeap;

// Helper function to swap two cells in the heap
static void swap(Cell** a, Cell** b) {
    Cell* temp = *a;
    *a = *b;
    *b = temp;
}

// Helper function to maintain heap property after insertion (bottom-up heapify)
static void heapify_up(CellHeap* heap, size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (heap->heap[parent]->coord.row <= heap->heap[index]->coord.row) {
            break; // Parent is smaller, heap is valid
        }
        swap(&heap->heap[parent], &heap->heap[index]);
        index = parent;
    }
}

// Helper function to maintain heap property after removal (top-down heapify)
static void heapify_down(CellHeap* heap, size_t index) {
    size_t left_child, right_child, smallest;

    while (1) {
        left_child = 2 * index + 1;
        right_child = 2 * index + 2;
        smallest = index;

        if (left_child < heap->size &&
            heap->heap[left_child]->coord.row < heap->heap[smallest]->coord.row) {
            smallest = left_child;
        }
        if (right_child < heap->size &&
            heap->heap[right_child]->coord.row < heap->heap[smallest]->coord.row) {
            smallest = right_child;
        }
        if (smallest == index) break;

        swap(&heap->heap[index], &heap->heap[smallest]);
        index = smallest;
    }
}

// Create a new heap-based CellSet
CellHeap* create_cell_heap(size_t initial_capacity) {
    CellHeap* heap = (CellHeap*)malloc(sizeof(CellHeap));
    heap->capacity = initial_capacity > INITIAL_HEAP_CAPACITY ? initial_capacity : INITIAL_HEAP_CAPACITY;
    heap->size = 0;
    heap->heap = (Cell**)malloc(sizeof(Cell*) * heap->capacity);
    return heap;
}

// Resize heap if needed
static void resize_cell_heap(CellHeap* heap) {
    heap->capacity *= 2;
    heap->heap = (Cell**)realloc(heap->heap, sizeof(Cell*) * heap->capacity);
}

// Insert a new cell into the heap
void cell_heap_add(CellHeap* heap, Cell* cell) {
    if (heap->size == heap->capacity) {
        resize_cell_heap(heap);
    }
    heap->heap[heap->size] = cell;
    heapify_up(heap, heap->size);
    heap->size++;
}

// Remove the minimum element (smallest row value cell) from the heap
Cell* cell_heap_remove_min(CellHeap* heap) {
    if (heap->size == 0) return NULL;

    Cell* min_cell = heap->heap[0];
    heap->size--;
    heap->heap[0] = heap->heap[heap->size];
    heapify_down(heap, 0);

    return min_cell;
}

// Find a cell in the heap (linear search since heap is not ordered like BST)
bool cell_heap_contains(CellHeap* heap, Cell* cell) {
    for (size_t i = 0; i < heap->size; i++) {
        if (heap->heap[i] == cell) {
            return true;
        }
    }
    return false;
}

// Remove a specific cell from the heap
bool cell_heap_remove(CellHeap* heap, Cell* cell) {
    for (size_t i = 0; i < heap->size; i++) {
        if (heap->heap[i] == cell) {
            heap->heap[i] = heap->heap[heap->size - 1]; // Replace with last element
            heap->size--;
            heapify_down(heap, i);
            return true;
        }
    }
    return false;
}

// Destroy the heap and free memory
void destroy_cell_heap(CellHeap* heap) {
    free(heap->heap);
    free(heap);
}


typedef struct CellNode {
    Cell* cell;
    struct CellNode* next;
} CellNode;

typedef struct {
    CellNode* head;
    CellNode* tail;
    size_t size;
} CellList;

CellList* create_cell_list() {
    CellList* list = (CellList*)malloc(sizeof(CellList));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void cell_list_add(CellList* list, Cell* cell) {
    CellNode* new_node = (CellNode*)malloc(sizeof(CellNode));
    new_node->cell = cell;
    new_node->next = NULL;

    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }
    
    list->tail = new_node;
    list->size++;
}

void destroy_cell_list(CellList* list) {
    CellNode* current = list->head;
    while (current) {
        CellNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    free(list);
}