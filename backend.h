#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "datastructures.h"

//Error codes
typedef enum {
    OK,
    ERR_INVALID_CELL,
    ERR_INVALID_FORMULA,
    ERR_CIRCULAR_REF,
    // ERR_INVALID_OPERATION,
    // ERR_INVALID_VALUE,
    // ERR_INVALID_ARGUMENT,
    // ERR_INVALID_FUNCTION,
    ERR_INVALID_REF,
    ERR_INVALID_RANGE,
    // ERR_INVALID_EXPRESSION,
    // ERR_INVALID_OPERATION,
    // ERR_INVALID_CELLREF,
    // ERR_INVALID_NODE,
    // ERR_INVALID_AST,
    // ERR_INVALID_SPREADSHEET,
    // ERR_INVALID_DEPENDENCY_GRAPH,
    ERR_DIV_ZERO
} CalcError;    

// initialize or end spreadsheeet
Spreadsheet *sheet_create(int rows, int cols);
void sheet_destroy(Spreadsheet *spreadsheet);

//cell operations
CalcError sheet_set_value(Spreadsheet *sheet, int row, int col, int value);
CalcError sheet_set_formula(Spreadsheet *sheet, int row, int col, const char *formula);

//Recalculate the value of the cell and all dependent cells
void sheet_recalculate(Spreadsheet *sheet, int row, int col);

// dependency management
bool sheet_check_circular(Spreadsheet *sheet, CellRef start);

#endif
