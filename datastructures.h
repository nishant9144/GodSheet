#ifndef BACKEND_H
#define BACKEND_H


#include <stdbool.h>
int col_to_index(const char *col_str);


//cordinate of cell (row,column)
typedef struct
{
    int row;
    int col;
} CellRef;

// formula AST node type

typedef enum{
    VAL_CONST,
    VAL_CELL,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    FUNC_MIN,
    FUNC_MAX,
    FUNC_SUM,
    FUNC_AVG,
    FINC_SLEEP,
    FUNC_STDEV,
} NodeType;


typedef enum {
    TOK_NUMBER,
    TOK_CELL,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_EOF,
    TOK_INVALID
} TokenType;

typedef struct {
    TokenType type;
    int num_value;
    CellRef cell_value;
    const char *start;
    size_t length;
} Token;

// Abstract Syntax Tree (AST) node
typedef struct ASTNode {
    NodeType type;
    int value;              // For the constants
    CellRef cellref;        // For the cell references
    struct ASTNode *left;   // Left operand or first argument
    struct ASTNode *right;  // Right operand or second argument
} ASTNode;

// Cell content 

typedef struct {
    int value;                //current value or for the constants     
    ASTNode *formula;         // Parsed formula(Null if no formula)  
    CellRef *dependencies;    // List of cells that depend on this cell or formula
    int deep_count;           // Number of dependencies
} Cell;

// Spreadsheet structure

typedef struct {
    int rows;                   // Number of rows
    int cols;                   // Number of columns
    Cell **cells;               // 2D array of cells
    bool **dependency_graph;    // Adjacency matrix of dependencies
} Spreadsheet;

#endif
