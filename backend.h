#ifndef BACKEND_H
#define BACKEND_H

#include "spreadsheet.h"

void destroy_spreadsheet(Spreadsheet *sheet);
void recalculate(Spreadsheet *sheet);
int evaluate_expression(Spreadsheet *sheet, int row, int col, const char *formula, CalcStatus *status);
void editCell(Spreadsheet *sheet);

#endif // BACKEND_H