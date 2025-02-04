#ifndef BACKEND_H
#define BACKEND_H

#include "spreadsheet.h"

void destroy_spreadsheet(Spreadsheet *sheet);
void recalculate(Spreadsheet *sheet);
int evaluate_expression(Spreadsheet *sheet, int row, int col, const char *formula, CalcStatus *status);
void editCell(Spreadsheet *sheet);
void collect_dependencies(ExprNode *node, Spreadsheet *sheet, int dest_row, int dest_col);
bool detect_cycle(Spreadsheet *sheet, int row, int col);
void mark_dependents_for_recalc(Spreadsheet *sheet, int row, int col);

#endif // BACKEND_H