#ifndef SHEET_H
#define SHEET_H
#include <stdbool.h>
#include <stdlib.h>

#define ERR -99999999
#define DIV_BY_ZERO -99999998
#define INITIAL_HEAP_CAPACITY 10

// Define cell coordinates structure
typedef struct {
    int row;    // 1-999
    char col[4]; // A-ZZZ
} CellCoord;

// Forward declaration for circular dependency
struct Cell;

// Heap-based dependency tracking structure
typedef struct {
    Cell** heap;       // Array of Cell pointers
    size_t size;       // Number of elements in heap
    size_t capacity;   // Max capacity before resizing
} CellHeap;

// Define the cell structure
typedef struct Cell {
    CellCoord coord;
    int value;
    char* formula;  // Store the original formula/expression
    bool needs_update;
    bool has_error;
    CellHeap* dependencies;  // Cells this cell depends on
    CellHeap* dependents;    // Cells that depend on this cell
} Cell;

// Define the spreadsheet structure
typedef struct {
    int rows;
    int cols;
    Cell*** cells;  // 2D array of cell pointers
} Spreadsheet;

// CellHeap operations
CellHeap* create_cell_heap(size_t initial_capacity);
void cell_heap_add(CellHeap* heap, Cell* cell);
Cell* cell_heap_remove_min(CellHeap* heap);
bool cell_heap_contains(CellHeap* heap, Cell* cell);
bool cell_heap_remove(CellHeap* heap, Cell* cell);
void destroy_cell_heap(CellHeap* heap);

// Core functions for dependency management
bool update_dependencies(Cell* curr_cell, CellHeap* new_dependencies);
bool check_circular_dependencies(Cell* curr_cell, CellHeap* visited, CellHeap* recursion_stack);
void update_dependents(Cell* curr_cell);
void topo_sort_util(Cell* cell, CellHeap* visited, CellList* topo_order);

// Helper functions
Cell* create_cell(int row, const char* col);
void destroy_cell(Cell* cell);
Spreadsheet* create_spreadsheet(int rows, int cols);
void destroy_spreadsheet(Spreadsheet* sheet);

// Linked List for ordered processing of cells
typedef struct CellNode {
    Cell* cell;
    struct CellNode* next;
} CellNode;

typedef struct {
    CellNode* head;
    CellNode* tail;
    size_t size;
} CellList;

CellList* create_cell_list();
void cell_list_add(CellList* list, Cell* cell);
void destroy_cell_list(CellList* list);

// Helper functions
Cell* create_cell(int row, const char* col);
void destroy_cell(Cell* cell);
Spreadsheet* create_spreadsheet(int rows, int cols);
void destroy_spreadsheet(Spreadsheet* sheet);

#endif