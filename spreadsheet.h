#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <sys/time.h>
#include <stdbool.h>
#include "datastructures.h"

typedef struct {
    int row;  // 0-based row index
    int col;  // 0-based column index
} CellRef;


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
    TYPE_EMPTY,
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

typedef struct Cell Cell;


struct Cell{
    int value;
    char *formula;
    CellType type;

    union{
        struct{
            Operation op;
            // Cell *operand1;  // For arithmetic expressions only.
            // Cell *operand2;
            int constant;
        } arithmetic;

        struct{
            char *func_name;
            Cell **range;
            int range_size;
        } function;
        Cell *ref; // NEW: For direct cell reference (TYPE_REFERENCE)
    } op_data;
    
    // Dependency management
    CellRefSet *dependents;   // Cells that depend on this one
    int dep_count;
    CellRefSet *dependencies; // Cells this cell depends on
    int depd_count;

    // For error handling
    bool has_error;
    char *error_msg;

    // For topological sort
    bool visited;   // For cycle detection
    bool in_stack;  // For cycle detection
    int topo_order; // Topological order
};

typedef struct
{
    Cell **cells;
    int totalRows;
    int totalCols;
    int scroll_row;
    int scroll_col;
    int mode;
    int cursorRow;
    int cursorCol;
    bool output_enabled;
    struct timeval last_cmd_time;
    CalcStatus last_status;
} Spreadsheet;

// Utility Functions
int cellref_compare(const CellRef *a, const CellRef *b);

#endif
