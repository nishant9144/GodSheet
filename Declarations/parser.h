#ifndef PARSER_H
#define PARSER_H

#include "header.h"
#include "ds.h"

int parse_formula(Spreadsheet *sheet, Cell *cell, const char *formula, bool *need_new_dep, PairOfPair *new_pairs);
void process_command(Spreadsheet *sheet, char *input);
int check_constant_or_cell_address(const char *str, int *constant_value, int *row, int *col, Spreadsheet* sheet);

Operation char_to_operation(char c);


#endif
