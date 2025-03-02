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
};

// AVL Tree Node (32)
struct AVLNode {
    Pair pair;
    struct AVLNode* left;
    struct AVLNode* right;
    unsigned char height;
}__attribute__((packed));

// Pair of Pair structure(8)
typedef struct {
    Pair first;
    Pair second;
} PairOfPair;

// Cell structure definition(40)
struct Cell {
    int value;   //(32)
    short row;   //(10.5)
    short col;   //(15.5)
    int topo_order; //(32)
    char type;  //(2)
    char cell_state; //(2) 
    bool is_sleep; //(1)
    bool has_error; //(1)
    union {
        struct {  
            Operation op; //(3)
            int constant; //(32)
        } arithmetic;

        struct {  
            char func_name; //(4)
        } function;
    } op_data;
    
    AVLNode* dependents;  //(64)
    PairOfPair dependencies; //(52)
}__attribute__((packed));


// Converting it to (32)
// value, topo_order, node* remains intact - (16 bytes)
// is-1(1) + row(10)+col(15)+state(2)+type(2)+is_sleep(1)+has_error(1) = 32 bits = (4 bytes)
// Operation code ko bahar nikal ke dependencies ke sath bitmask kar dete hain
// 52+3 = 55 -> 64 (8 bytes)
// andar ke op_data ka size 32 = (4 bytes)
// jab -1 mark karna hoga to fir saare bits 1 kardenge row ke so that we can identify ki -1 hai ya nahi

typedef struct Cell1{
    int value;      //(4)
    int topo_order; //(4)
    AVLNode* dependents; //(8)
    union {
        struct {  
            int constant; //(4)
        } arithmetic;

        struct {  
            char func_name; //(1)
        } function;
    } op_data;
    u_int32_t nrctsle; //(4)
    uint64_t nrcnrco;  //(8)
}__attribute__((packed)) Cell1;


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
void topologic_util(Cell* currcell, Vector* adjList, char* visited, Vector* sorted, Spreadsheet* sheet);
void topological_sort(Vector* adjList, int numVertices, Pair** cell_map, Vector* result, Spreadsheet* sheet);

void create_cell(short row, short col, Cell* Cell);
void free_cell(Cell* cell);

short colNameToNumber(const char *colName);
void colNumberToName(short colNumber, char *colName);
// void print_cell(Cell* cell);
Spreadsheet* create_spreadsheet(short rows, short cols);
void print_spreadsheet(Spreadsheet* sheet);
void free_spreadsheet(Spreadsheet* sheet);

#endif // DATASTRUCTURES_H