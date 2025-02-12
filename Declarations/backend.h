#ifndef BACKEND_H
#define BACKEND_H

#include "header.h"
#include "ds.h"

#define DIV_BY_ZERO -999999
#define MAX_CELL_LENGTH 50

void print_cell(Cell* cell);
void print_dependents(Cell* cell);
int update_dependencies(Cell* curr_cell, Set* new_dependencies);
bool detect_cycle_dfs(Cell* curr_cell, Set* visited, Set* recursion_stack);
bool check_circular_dependencies(Cell* curr_cell);
void collect_dependents(Cell* curr_cell, Set* affected_cells);
void update_dependents(Cell* curr_cell);
Set* createDependenciesSet(Vector* List);
void editCell(Spreadsheet *sheet);
int evaluate_cell(Cell *cell);

#endif