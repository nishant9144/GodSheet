#include "spreadsheet.h"
#include "datastructures.h"

// ====================
// CellRef Comparison
// ====================
int cellref_compare(const CellRef *a, const CellRef *b) {
    if (a->row != b->row) return a->row - b->row;
    return a->col - b->col;
}

// ====================
// Dependency Management
// ====================
void add_dependency(Spreadsheet *sheet, CellRef src, CellRef dest) {
    // Initialize sets if NULL
    if (!sheet->cells[src.row][src.col].dependents) {
        sheet->cells[src.row][src.col].dependents = cellref_set_create(cellref_compare);
    }
    if (!sheet->cells[dest.row][dest.col].dependencies) {
        sheet->cells[dest.row][dest.col].dependencies = cellref_set_create(cellref_compare);
    }
    
    cellref_set_insert(sheet->cells[src.row][src.col].dependents, dest);
    cellref_set_insert(sheet->cells[dest.row][dest.col].dependencies, src);
}

// ====================
// Spreadsheet Creation
// ====================
Spreadsheet* create_spreadsheet(int rows, int cols) {
    Spreadsheet *sheet = malloc(sizeof(Spreadsheet));
    // ... existing initialization code ...

    // Initialize cells with dependency sets
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sheet->cells[i][j].dependents = cellref_set_create(cellref_compare);
            sheet->cells[i][j].dependencies = cellref_set_create(cellref_compare);
        }
    }
    return sheet;
}