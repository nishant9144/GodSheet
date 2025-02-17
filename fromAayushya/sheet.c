#include "sheet.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool update_dependencies(Cell* curr_cell, CellHeap* new_dependencies) {
    CellHeap* old_deps = curr_cell->dependencies;

    // Remove curr_cell from old dependencies' dependents heap
    while (old_deps->size > 0) {
        Cell* dep = cell_heap_remove_min(old_deps);
        cell_heap_remove(dep->dependents, curr_cell);
    }

    // Add curr_cell to new dependencies' dependents heap
    for (size_t i = 0; i < new_dependencies->size; i++) {
        Cell* dep = new_dependencies->heap[i];
        cell_heap_add(dep->dependents, curr_cell);
    }

    // Check for circular dependencies
    curr_cell->dependencies = new_dependencies;
    CellHeap* visited = create_cell_heap(INITIAL_HEAP_CAPACITY);
    CellHeap* recursion_stack = create_cell_heap(INITIAL_HEAP_CAPACITY);
    
    bool has_cycle = !check_circular_dependencies(curr_cell, visited, recursion_stack);
    
    if (has_cycle) {
        printf("Error: Circular dependency detected.\n");

        // Rollback changes
        while (new_dependencies->size > 0) {
            Cell* dep = cell_heap_remove_min(new_dependencies);
            cell_heap_remove(dep->dependents, curr_cell);
        }

        curr_cell->dependencies = old_deps;
        destroy_cell_heap(visited);
        destroy_cell_heap(recursion_stack);
        destroy_cell_heap(new_dependencies);
        return false;
    }

    // Cleanup
    destroy_cell_heap(visited);
    destroy_cell_heap(recursion_stack);
    destroy_cell_heap(old_deps);
    return true;
}

bool check_circular_dependencies(Cell* curr_cell, CellHeap* visited, CellHeap* recursion_stack) {
    if (curr_cell == NULL) return true;

    if (cell_heap_contains(recursion_stack, curr_cell)) {
        return false; // Cycle detected
    }

    if (cell_heap_contains(visited, curr_cell)) {
        return true;
    }

    cell_heap_add(visited, curr_cell);
    cell_heap_add(recursion_stack, curr_cell);

    for (size_t i = 0; i < curr_cell->dependencies->size; i++) {
        if (!check_circular_dependencies(curr_cell->dependencies->heap[i], visited, recursion_stack)) {
            return false;
        }
    }

    cell_heap_remove(recursion_stack, curr_cell);
    return true;
}

void topo_sort_util(Cell* cell, CellHeap* visited, CellList* topo_order) {
    if (cell_heap_contains(visited, cell)) {
        return;
    }
    
    cell_heap_add(visited, cell);

    for (size_t i = 0; i < cell->dependents->size; i++) {
        Cell* dependent = cell->dependents->heap[i];
        if (dependent->needs_update) {
            topo_sort_util(dependent, visited, topo_order);
        }
    }

    cell_list_add(topo_order, cell);
}

void update_dependents(Cell* curr_cell) {
    CellList* topo_order = create_cell_list();
    CellHeap* visited = create_cell_heap(INITIAL_HEAP_CAPACITY);
    
    topo_sort_util(curr_cell, visited, topo_order);

    CellNode* node = topo_order->head;
    while (node) {
        Cell* cell = node->cell;
        cell->has_error = false;  // Reset error state

        double result = evaluate_formula(cell);
        
        if (result == DIV_BY_ZERO) {
            cell->value = ERR;
            cell->has_error = true;
        } else {
            cell->value = result;
        }

        cell->needs_update = false;
        node = node->next;
    }

    destroy_cell_list(topo_order);
    destroy_cell_heap(visited);
}

Cell* create_cell(int row, const char* col) {
    Cell* cell = malloc(sizeof(Cell));
    cell->coord.row = row;
    strncpy(cell->coord.col, col, 3);
    cell->coord.col[3] = '\0';
    cell->value = 0;
    cell->formula = NULL;
    cell->needs_update = false;
    cell->has_error = false;
    cell->dependencies = create_cell_heap(INITIAL_HEAP_CAPACITY);
    cell->dependents = create_cell_heap(INITIAL_HEAP_CAPACITY);
    return cell;
}

void destroy_cell(Cell* cell) {
    free(cell->formula);
    destroy_cell_heap(cell->dependencies);
    destroy_cell_heap(cell->dependents);
    free(cell);
}

Spreadsheet* create_spreadsheet(int rows, int cols) {
    Spreadsheet* sheet = malloc(sizeof(Spreadsheet));
    sheet->rows = rows;
    sheet->cols = cols;
    
    sheet->cells = malloc(sizeof(Cell**) * rows);
    for (int i = 0; i < rows; i++) {
        sheet->cells[i] = malloc(sizeof(Cell*) * cols);
        for (int j = 0; j < cols; j++) {
            char col[4] = {0};
            int temp = j;
            int idx = 0;
            do {
                col[idx++] = 'A' + (temp % 26);
                temp = temp / 26 - 1;
            } while (temp >= 0 && idx < 3);
            
            for (int k = 0; k < idx/2; k++) {
                char t = col[k];
                col[k] = col[idx-1-k];
                col[idx-1-k] = t;
            }
            
            sheet->cells[i][j] = create_cell(i + 1, col);
        }
    }
    
    return sheet;
}

void destroy_spreadsheet(Spreadsheet* sheet) {
    for (int i = 0; i < sheet->rows; i++) {
        for (int j = 0; j < sheet->cols; j++) {
            destroy_cell(sheet->cells[i][j]);
        }
        free(sheet->cells[i]);
    }
    free(sheet->cells);
    free(sheet);
}
