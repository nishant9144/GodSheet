#ifndef BACKEND_H
#define BACKEND_H

#include "header.h"
#include "ds.h"

#define DIV_BY_ZERO -999999

void print_cell(Cell *cell);
void print_dependents(Cell *cell);
int update_dependencies(Cell *curr_cell, bool need_new_dep, PairOfPair *new_pairs, Spreadsheet *sheet, Cell cellcopy);
bool detect_cycle_dfs(Cell *cell, Spreadsheet *sheet, Vector *bin);
bool check_circular_dependencies(Cell *curr_cell, Spreadsheet *sheet);
// void collect_dependents(Cell *curr_cell, Set *affected_cells, Spreadsheet *sheet);
void update_dependents(Cell *curr_cell, Spreadsheet *sheet);
// void editCell(Spreadsheet *sheet);
int evaluate_cell(Cell *cell, Spreadsheet *sheet);

#endif