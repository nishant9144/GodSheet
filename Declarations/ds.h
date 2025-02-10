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

// Data structure definitions
struct Vector {
    Cell** data;
    size_t size;
    size_t capacity;
};

struct Queue {
    Cell** data;
    size_t capacity;
    size_t size;
    size_t front;
    size_t rear;
};

struct Stack {
    Cell** data;
    size_t size;
    size_t capacity;
};

struct AVLNode {
    Cell* cell;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
};

struct Set {
    AVLNode* root;
};

// Cell structure definition
struct Cell {
    int value;          
    int row; // -> can fix to 3 field.
    char* formula; 
    char* col;  /// max of 3 fields
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
typedef struct {
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
} Spreadsheet;

// Iterator structures
typedef struct {
    Vector* vector;
    size_t index;
} VectorIterator;

typedef struct {
    Queue* queue;
    size_t index;
} QueueIterator;

typedef struct {
    Stack* stack;
    size_t index;
} StackIterator;

typedef struct {
    AVLNode** stack;
    size_t capacity;
    size_t top;
} SetIterator;

// Function declarations
void vector_init(Vector* vector);
void vector_push_back(Vector* vector, Cell* value);
void vector_free(Vector* vector);

void vector_iterator_init(VectorIterator* iterator, Vector* vector);
int vector_iterator_has_next(VectorIterator* iterator);
Cell* vector_iterator_next(VectorIterator* iterator);

void queue_init(Queue* queue, size_t capacity);
int queue_is_full(Queue* queue);
int queue_is_empty(Queue* queue);
void queue_enqueue(Queue* queue, Cell* value);
Cell* queue_dequeue(Queue* queue);
void queue_free(Queue* queue);

void queue_iterator_init(QueueIterator* iterator, Queue* queue);
int queue_iterator_has_next(QueueIterator* iterator);
Cell* queue_iterator_next(QueueIterator* iterator);

void stack_init(Stack* stack);
void stack_push(Stack* stack, Cell* value);
Cell* stack_pop(Stack* stack);
void stack_free(Stack* stack);

void stack_iterator_init(StackIterator* iterator, Stack* stack);
int stack_iterator_has_next(StackIterator* iterator);
Cell* stack_iterator_next(StackIterator* iterator);

void set_init(Set* set);
void set_add(Set* set, Cell* cell);
Cell* set_find(Set* set, Cell* cell);
void set_remove(Set* set, Cell* cell);
void set_free(Set* set);

void set_iterator_init(SetIterator* iterator, Set* set);
int set_iterator_has_next(SetIterator* iterator);
Cell* set_iterator_next(SetIterator* iterator);
void set_iterator_free(SetIterator* iterator);



int compare_cells_position(Cell* cell1, Cell* cell2);
void topological_sort_util(Cell* cell, Set* adjList, Set* visited, Vector* sorted);
Vector topological_sort(Set* adjList, int numVertices, Cell** cell_map);


Cell* create_cell(int row, int col);
void free_cell(Cell* cell);

int colNameToNumber(const char *colName);
void colNumberToName(int colNumber, char *colName);
// void print_cell(Cell* cell);
Spreadsheet* create_spreadsheet(int rows, int cols);
void print_spreadsheet(Spreadsheet* sheet);
void free_spreadsheet(Spreadsheet* sheet);

#endif // DATASTRUCTURES_H