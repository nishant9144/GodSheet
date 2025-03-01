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
    MIN, //->A
    MAX, //->B
    AVG, //->C
    SUM, //->D
    STDEV, //->E
} FunctionName;

typedef enum
{
    TYPE_CONSTANT,   // C
    TYPE_REFERENCE,  // R
    TYPE_ARITHMETIC, // A
    TYPE_FUNCTION,   // F
} CellType;

typedef enum 
{
    UNVISITED,  // U
    VISITING,   // V
    SAFE,       // S
    NOT_USED    // N
} State;

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
    short i;
    short j;
} Pair;

// Vector implementation
struct Vector{
    size_t size;
    size_t capacity;
    Pair* data;
    // Spreadsheet* sheet;
};

// Queue implementation
struct Queue{
    size_t capacity;
    size_t front;
    size_t rear;
    size_t size;
    Pair* data;
    // Spreadsheet* sheet;
};

// Stack implementation
struct Stack{
    size_t size;
    size_t capacity;
    Pair* data;
    // Spreadsheet* sheet;
};

// AVL Tree Node
struct AVLNode {
    Pair pair;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
};

// // Set implementation
// struct Set{
//     AVLNode* root;
//     // Spreadsheet* sheet;
//     char type;
// };

// Pair of Pair structure
typedef struct {
    Pair first;
    Pair second;
} PairOfPair;

// Cell structure definition
struct Cell {
    int value;          
    short row; 
    short col; 
    int topo_order;
    char type; 
    char cell_state;  
    bool is_sleep;
    bool has_error;
    union {
        struct {  
            Operation op;
            int constant;
        } arithmetic;

        struct {  
            char func_name;
        } function;
    } op_data;
    
    AVLNode* dependents;
    PairOfPair dependencies;
};

// Spreadsheet structure
struct Spreadsheet{
    Cell **cells;
    int totalRows;
    int totalCols;
    int scroll_row;
    int scroll_col;
    CalcStatus last_status;
    struct timeval last_cmd_time;
    bool output_enabled;
    double last_processing_time;
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

// typedef struct {
//     Set* set;
//     size_t capacity;
//     AVLNode** stack;
//     size_t top;
// } SetIterator;


// Function declarations
void vector_init(Vector* vector);
void vector_push_back(Vector* vector, short row, short col);
void vector_free(Vector* vector);

void vector_iterator_init(VectorIterator* iterator, Vector* vector);
bool vector_iterator_has_next(VectorIterator* iterator);
Pair* vector_iterator_next(VectorIterator* iterator);

void queue_init(Queue* queue, size_t capacity);
bool queue_is_full(Queue* queue);
bool queue_is_empty(Queue* queue);
void queue_enqueue(Queue* queue, short row, short col);
Pair* queue_dequeue(Queue* queue);
void queue_free(Queue* queue);

void queue_iterator_init(QueueIterator* iterator, Queue* queue);
bool queue_iterator_has_next(QueueIterator* iterator);
Pair* queue_iterator_next(QueueIterator* iterator);

void stack_init(Stack* stack);
void stack_push(Stack* stack, short row, short col);
Pair* stack_pop(Stack* stack);
void stack_free(Stack* stack);

void stack_iterator_init(StackIterator* iterator, Stack* stack);
bool stack_iterator_has_next(StackIterator* iterator);
Pair* stack_iterator_next(StackIterator* iterator);

// void set_init(Set* set);
// void set_add(Set* set, short row, short col);
// Pair* set_find(Set* set, short row, short col);
// void set_remove(Set* set, short row, short col);
// void set_free(Set* set);

// void set_iterator_init(SetIterator* iterator, Set* set);
// bool set_iterator_has_next(SetIterator* iterator);
// Pair* set_iterator_next(SetIterator* iterator);
// void set_iterator_free(SetIterator* iterator);


AVLNode* avl_create_node(short row, short col);
AVLNode* avl_insert(AVLNode* root, short row, short col);
Pair* avl_find(AVLNode* root, short row, short col);
AVLNode* avl_remove(AVLNode* root, short row, short col);
void avl_free(AVLNode* root);

// void topological_sort_util(Cell* cell, Set* adjList, Set* visited, Vector* sorted, Spreadsheet *sheet);
void collect_traverse_avl_tree(AVLNode* node, Cell* current_cell, AVLNode*** adjList, AVLNode** visited, Vector* sorted, Spreadsheet* sheet);
void collect_traverse_topo(Cell* cell, AVLNode*** adjList, AVLNode** visited, Vector* sorted, Spreadsheet* sheet);
void topological_sort(AVLNode*** adjList, int numVertices, Cell** cell_map, Vector* result, Spreadsheet* sheet);

void create_cell(short row, short col, Cell* Cell);
void free_cell(Cell* cell);

short colNameToNumber(const char *colName);
void colNumberToName(short colNumber, char *colName);
// void print_cell(Cell* cell);
Spreadsheet* create_spreadsheet(short rows, short cols);
void print_spreadsheet(Spreadsheet* sheet);
void free_spreadsheet(Spreadsheet* sheet);

#endif // DATASTRUCTURES_H