#ifndef PARSER_H
#define PARSER_H

#include "spreadsheet.h"

void process_command(Spreadsheet *sheet, char *input);
int parse_formula(Spreadsheet *sheet, Cell *cell, const char *formula);
int evaluate_expression(const char *expr, int *result);

#endif
