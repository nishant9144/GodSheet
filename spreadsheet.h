#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <sys/time.h>
#include <stdbool.h>

#define VIEWPORT_ROWS 10
#define VIEWPORT_COLS 10
#define CELL_WIDTH 12
#define MAX_CELL_LENGTH 50
#define CELL_PADDING 1 // Space between cells
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
    bool isFormula;
    char formula[MAX_FORMULA_LEN];
    DependencyNode *dependents;
    DependencyNode *dependencies;
    bool needs_recalc;
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

// Function prototypes
void add_dependency(Spreadsheet *sheet, int src_row, int src_col, int target_row, int target_col);
bool detect_cycle(Spreadsheet *sheet, int row, int col);
void clear_dependencies(Spreadsheet *sheet, int row, int col);
int evaluate_expression(Spreadsheet *sheet, int row, int col, const char *formula, CalcStatus *status);
void recalculate(Spreadsheet *sheet);
Spreadsheet *create_spreadsheet(int rows, int cols);
void destroy_spreadsheet(Spreadsheet *sheet);

#endif
