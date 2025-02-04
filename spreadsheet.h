#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <sys/time.h>
#include <stdbool.h>

#define VIEWPORT_ROWS 10
#define VIEWPORT_COLS 10
#define CELL_WIDTH 12
#define MAX_CELL_LENGTH 50
#define CELL_PADDING 1
#define MAX_FORMULA_LEN 100
#define MAX_DEPS 100
#define SLEEP_TIME_MULTIPLIER 1000000
#define MAX_ROWS 999
#define MAX_COLS 18278
#define VIEW_MODE 0
#define EDIT_MODE 1
#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_RIGHT 'C'
#define ARROW_LEFT 'D'
#define ESC '\x1b'

// Terminal control sequences
#define CLEAR_SCREEN "\x1b[2J\x1b[H"
#define HIDE_CURSOR "\x1b[?25l"
#define SHOW_CURSOR "\x1b[?25h"

typedef enum
{
    STATUS_OK,
    ERR_INVALID_CELL,
    ERR_FORMULA_NOT_REFERENCED,
    ERR_CIRCULAR_REF,
    ERR_DIV_ZERO,
    ERR_INVALID_RANGE,
    ERR_SYNTAX,
    ERR_OVERFLOW
} CalcStatus;

typedef struct DependencyNode
{
    int row;
    int col;
    struct DependencyNode *next;
} DependencyNode;

typedef struct
{
    int content;
    char *formula;                // Will have to use malloc for formula
    ExprNode *expr_tree;          // Parsed expression tree
    DependencyNode *dependents;   // Cells that depend on this one
    DependencyNode *dependencies; // Cells this cell depends on
    bool needs_recalc;
    bool has_error;
    char error_msg[64];
} Cell;

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
    struct timeval last_cmd_time;
    CalcStatus last_status;
} Spreadsheet;

typedef enum
{
    NODE_CONSTANT,
    NODE_REFERENCE,
    NODE_OPERATOR,
    NODE_FUNCTION,
    NODE_RANGE
} ExprNodeType;

// typedef struct ExprNode {
//     ExprNodeType type;
//     union {
//         int constant;
//         struct { int row; int col; } reference;  // Converted cell ref (e.g., A1 → 0,0)
//         struct { char op; struct ExprNode *left, *right; } operation;
//         struct {
//             char *name;
//             struct ExprNode **args;
//             int args_count;
//         } function;
//         struct { struct ExprNode *start; struct ExprNode *end; } range;
//     };
// } ExprNode;

typedef struct ExprNode {
    ExprNodeType type;
    union {
        // For constants (NODE_CONSTANT)
        int constant;
        
        // For cell references (NODE_REFERENCE)
        struct {
            int row;  // 0-based row index
            int col;  // 0-based column index
        } reference;
        
        // For operators (NODE_OPERATOR)
        struct {
            char op;             // '+', '-', '*', '/'
            struct ExprNode *left;
            struct ExprNode *right;
        } operation;
        
        // For functions (NODE_FUNCTION)
        struct {
            char *name;          // Function name ("SUM", "AVG", etc.)
            struct ExprNode **args; // Array of arguments (only ranges supported)
            int args_count;
        } function;
        
        // For ranges (NODE_RANGE)
        struct {
            struct ExprNode *start; // Start cell (NODE_REFERENCE)
            struct ExprNode *end;   // End cell (NODE_REFERENCE)
        } range;
    };
} ExprNode;

// Function prototypes
void add_dependency(Spreadsheet *sheet, int src_row, int src_col, int target_row, int target_col);
bool detect_cycle(Spreadsheet *sheet, int row, int col);
void clear_dependencies(Spreadsheet *sheet, int row, int col);
void recalculate(Spreadsheet *sheet);
Spreadsheet *create_spreadsheet(int rows, int cols);
void destroy_spreadsheet(Spreadsheet *sheet);

ExprNode *parse_expression(const char **input);
void free_expr_tree(ExprNode *node);
int eval_expression(Spreadsheet *sheet, int row, int col, CalcStatus *status);

#endif
