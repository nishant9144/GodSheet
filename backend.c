#include "backend.h"
#include "datastructures.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Helper: Convert column string (e.g., "AA") to 0-based index
static int col_to_index(const char *col_str) {
    int index = 0;
    while (*col_str) {
        index = index * 26 + (toupper(*col_str) - 'A' + 1);
        col_str++;
    }
    return index - 1;
}

// Helper: Parse cell reference like "A1"
static bool parse_cell_ref(const char *str, CellRef *ref) {
    char col_str[4] = {0};
    int i = 0;
    
    // Extract column
    while (isalpha(str[i]) && i < 3) {
        col_str[i] = str[i];
        i++;
    }
    if (i == 0) return false;
    
    // Extract row
    int row = atoi(str + i) - 1;
    if (row < 0) return false;
    
    ref->col = col_to_index(col_str);
    ref->row = row;
    return true;
}

// Create/destroy spreadsheet
Spreadsheet* sheet_create(int rows, int cols) {
    Spreadsheet *sheet = malloc(sizeof(Spreadsheet));
    sheet->rows = rows;
    sheet->cols = cols;
    
    // Allocate cells
    sheet->cells = malloc(rows * sizeof(Cell*));
    for (int i = 0; i < rows; i++) {
        sheet->cells[i] = calloc(cols, sizeof(Cell));
    }
    
    // Initialize dependency graph
    sheet->dependency_graph = calloc(rows * cols, sizeof(bool*));
    for (int i = 0; i < rows * cols; i++) {
        sheet->dependency_graph[i] = calloc(rows * cols, sizeof(bool));
    }
    
    return sheet;
}

void sheet_destroy(Spreadsheet *sheet) {
    // Free cells
    for (int i = 0; i < sheet->rows; i++) {
        for (int j = 0; j < sheet->cols; j++) {
            free(sheet->cells[i][j].dependencies);
        }
        free(sheet->cells[i]);
    }
    free(sheet->cells);
    
    // Free dependency graph
    for (int i = 0; i < sheet->rows * sheet->cols; i++) {
        free(sheet->dependency_graph[i]);
    }
    free(sheet->dependency_graph);
    
    free(sheet);
}

// Set cell value (triggers recalc)
CalcError sheet_set_value(Spreadsheet *sheet, int row, int col, int value) {
    if (row < 0 || row >= sheet->rows || col < 0 || col >= sheet->cols) {
        return ERR_INVALID_REF;
    }
    
    Cell *cell = &sheet->cells[row][col];
    
    // Clear existing formula
    if (cell->formula) {
        free(cell->formula);
        cell->formula = NULL;
    }
    
    cell->value = value;
    sheet_recalculate(sheet);
    return OK;
}

// Recursive formula evaluation
static int evaluate_ast(Spreadsheet *sheet, ASTNode *node, CalcError *err) {
    if (*err != OK) return 0;
    
    switch (node->type) {
        case VAL_CONST: return node->value;
        case VAL_CELL: {
            if (node->cellref.row >= sheet->rows || node->cellref.col >= sheet->cols) {
                *err = ERR_INVALID_REF;
                return 0;
            }
            return sheet->cells[node->cellref.row][node->cellref.col].value;
        }
        case OP_ADD: 
            return evaluate_ast(sheet, node->left, err) + evaluate_ast(sheet, node->right, err);
        case OP_SUB:
            return evaluate_ast(sheet, node->left, err) - evaluate_ast(sheet, node->right, err);
        case OP_MUL:
            return evaluate_ast(sheet, node->left, err) * evaluate_ast(sheet, node->right, err);
        case OP_DIV: {
            int denom = evaluate_ast(sheet, node->right, err);
            if (denom == 0) {
                *err = ERR_DIV_ZERO;
                return 0;
            }
            return evaluate_ast(sheet, node->left, err) / denom;
        }
        // Handle functions (MIN, MAX, etc.)
        default: {
            *err = ERR_INVALID_FORMULA;
            return 0;
        }
    }
}

// Trigger recalculation of dependent cells
void sheet_recalculate(Spreadsheet *sheet) {
    // Implement topological sort using dependency graph
    // For simplicity, this is a placeholder
    for (int i = 0; i < sheet->rows; i++) {
        for (int j = 0; j < sheet->cols; j++) {
            Cell *cell = &sheet->cells[i][j];
            if (cell->formula) {
                CalcError err = OK;
                int new_val = evaluate_ast(sheet, cell->formula, &err);
                if (err == OK) cell->value = new_val;
            }
        }
    }
}