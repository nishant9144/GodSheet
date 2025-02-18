#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "header.h"

// Forward declarations
typedef struct Cell Cell;
typedef struct Vector Vector;
typedef struct Queue Queue;
typedef struct Stack Stack;
typedef struct AVLNode AVLNode;
typedef struct Set Set;
typedef struct Spreadsheet Spreadsheet;

// Enums
typedef enum
{
    STATUS_OK,
    ERR_INVALID_CELL,
    ERR_FORMULA_NOT_REFERENCED,
    ERR_CIRCULAR_REF,
    ERR_DIV_ZERO,
    ERR_INVALID_RANGE,
    ERR_SYNTAX,
    ERR_OVERFLOW,
    ERR_CIRCULAR_REFERENCE
} CalcStatus;

typedef enum
{
    TYPE_CONSTANT,
    TYPE_REFERENCE,  // References another cell (e.g., =A1)
    TYPE_ARITHMETIC, // Simple arithmetic (e.g., =A1+1)
    TYPE_FUNCTION,   // Functions like MIN, MAX, etc.
} CellType;

typedef enum
{
    OP_NONE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} Operation;

// Basic pair structure needed internally
typedef struct {
    int i;
    int j;
} Pair;

// Vector implementation
struct Vector{
    size_t size;
    size_t capacity;
    Pair* data;
    Spreadsheet* sheet;
};

// Queue implementation
struct Queue{
    size_t capacity;
    size_t front;
    size_t rear;
    size_t size;
    Pair* data;
    Spreadsheet* sheet;
};

// Stack implementation
struct Stack{
    size_t size;
    size_t capacity;
    Pair* data;
    Spreadsheet* sheet;
};

// AVL Tree Node
struct AVLNode {
    Pair pair;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
};

// Set implementation
struct Set{
    AVLNode* root;
    Spreadsheet* sheet;
};

// Cell structure definition
struct Cell {
    int value;          
    int row; // -> can fix to 3 field.
    char* formula; 
    int col;  /// max of 3 fields
    CellType type;     // use bits 
    int topo_order;

    union {
        Cell* ref;
        struct {  
            Operation op;       // use bits
            int constant;      
            Cell* operand1;     
            Cell* operand2;
        } arithmetic;

        struct {  
            char* func_name;        
            int range_size;     
        } function;
    } op_data;
    
    
    Set* dependents;  
    Set* dependencies;
    bool is_sleep;
    bool has_error;
   
    // char* error_msg;    
    
    // bool visited;      
    // bool in_stack;     
        
};

// Spreadsheet structure
struct Spreadsheet{
    Cell **cells;
    int totalRows;
    int totalCols;
    int scroll_row;
    int scroll_col;
    // int mode;
    // int cursorRow;
    // int cursorCol;
    CalcStatus last_status;
    struct timeval last_cmd_time;
    bool output_enabled;
};

// Vector iterator
typedef struct {
    Vector* vector;
    size_t index;
} VectorIterator;


// Queue iterator
typedef struct {
    Queue* queue;
    size_t index;
} QueueIterator;

// Stack iterator
typedef struct {
    Stack* stack;
    size_t index;
} StackIterator;

typedef struct {
    Set* set;
    size_t capacity;
    AVLNode** stack;
    size_t top;
} SetIterator;


// Function declarations
void vector_init(Vector* vector, Spreadsheet* sheet);
void vector_push_back(Vector* vector, int row, int col);
void vector_free(Vector* vector);

void vector_iterator_init(VectorIterator* iterator, Vector* vector);
int vector_iterator_has_next(VectorIterator* iterator);
Cell* vector_iterator_next(VectorIterator* iterator);

void queue_init(Queue* queue, size_t capacity, Spreadsheet* sheet);
int queue_is_full(Queue* queue);
int queue_is_empty(Queue* queue);
void queue_enqueue(Queue* queue, int row, int col);
Cell* queue_dequeue(Queue* queue);
void queue_free(Queue* queue);

void queue_iterator_init(QueueIterator* iterator, Queue* queue);
int queue_iterator_has_next(QueueIterator* iterator);
Cell* queue_iterator_next(QueueIterator* iterator);

void stack_init(Stack* stack, Spreadsheet* sheet);
void stack_push(Stack* stack, int row, int col);
Cell* stack_pop(Stack* stack);
void stack_free(Stack* stack);

void stack_iterator_init(StackIterator* iterator, Stack* stack);
int stack_iterator_has_next(StackIterator* iterator);
Cell* stack_iterator_next(StackIterator* iterator);

void set_init(Set* set, Spreadsheet* sheet);
void set_add(Set* set, int row, int col);
Cell* set_find(Set* set, int row, int col);
void set_remove(Set* set, int row, int col);
void set_free(Set* set);

void set_iterator_init(SetIterator* iterator, Set* set);
int set_iterator_has_next(SetIterator* iterator);
Cell* set_iterator_next(SetIterator* iterator);
void set_iterator_free(SetIterator* iterator);



int compare_cells_position(Cell* cell1, Cell* cell2);
void topological_sort_util(Cell* cell, Set* adjList, Set* visited, Vector* sorted);
void topological_sort(Set* adjList, int numVertices, Cell** cell_map, Vector* result, Spreadsheet* sheet);


Cell* create_cell(int row, int col);
void free_cell(Cell* cell);

int colNameToNumber(const char *colName);
void colNumberToName(int colNumber, char *colName);
// void print_cell(Cell* cell);
Spreadsheet* create_spreadsheet(int rows, int cols);
void print_spreadsheet(Spreadsheet* sheet);
void free_spreadsheet(Spreadsheet* sheet);

#endif // DATASTRUCTURES_H