#ifndef PARSER_H
#define PARSER_H

#include "spreadsheet.h"

void process_command(Spreadsheet *sheet, char *input);
ExprNode *parse_formula(const char *formula, CalcStatus *status);
void free_expr_tree(ExprNode *node);

#endif
