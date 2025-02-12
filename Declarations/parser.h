#ifndef PARSER_H
#define PARSER_H

#include "header.h"
#include "ds.h"


static int check_constant_or_cell_address(const char *str, int *constant_value, int *row, int *col);
void evaluate_cell(Cell *cell);
static int default_ptr_cmp(const void *a, const void *b);
static int col_label_to_index(const char *label);
static int parse_cell_address(Spreadsheet *sheet, const char **input, int *row, int *col);
static int is_valid_formula(const char *formula);

// Convert operator character to Operation enum
static Operation char_to_operation(char c)
{
    switch (c)
    {
    case '+':
        return OP_ADD;
    case '-':
        return OP_SUB;
    case '*':
        return OP_MUL;
    case '/':
        return OP_DIV;
    default:
        return OP_NONE;
    }
}


static int parse_range(Spreadsheet *sheet, const char *range_str, Set* new_deps);
static int parse_arithmetic(Spreadsheet *sheet, Cell *target_cell, const char *formula, Set* new_deps);
static int parse_function(Spreadsheet *sheet, Cell *target_cell, const char *formula, Set* new_deps);
static int check_constant_or_cell_address(const char *str, int *constant_value, int *row, int *col);
int parse_formula(Spreadsheet *sheet, Cell *cell, const char *formula, Set* new_deps);
void process_command(Spreadsheet *sheet, char *input);
void evaluate_cell(Cell* cell);


// don't need these
void mark_for_recalculation(Spreadsheet *sheet, Cell *cell);
void recalculate_sheet(Spreadsheet *sheet);


#endif
