#ifndef BACKEND_H
#define BACKEND_H

#include "spreadsheet.h"

Spreadsheet *create_spreadsheet(int rows, int cols);
void recalculate(Spreadsheet *sheet);
void editCell(Spreadsheet *sheet);
void destroy_spreadsheet(Spreadsheet *sheet);

#endif