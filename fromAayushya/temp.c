#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME 10
#define ERR_VALUE -99999999 // Special value for error cases

// Cell structure
typedef struct Cell {
    char name[MAX_NAME];    // Cell name (e.g., "A1")
    int value;              // Cell value
    char* formula;          // Stores formula as a string
    struct Cell** dependencies; // List of dependencies
    int dep_count;
    struct Cell** dependents;   // List of dependents
    int depd_count;
    bool needs_update;
} Cell;

// Function to create a new cell
Cell* create_cell(const char* name) {
    Cell* new_cell = (Cell*)malloc(sizeof(Cell));
    strcpy(new_cell->name, name);
    new_cell->value = 0;
    new_cell->formula = NULL;
    new_cell->dependencies = NULL;
    new_cell->dependents = NULL;
    new_cell->dep_count = 0;
    new_cell->depd_count = 0;
    new_cell->needs_update = false;
    return new_cell;
}

// Function to add a dependency
void add_dependency(Cell* curr_cell, Cell* dep_cell) {
    curr_cell->dependencies = (Cell**)realloc(curr_cell->dependencies, (curr_cell->dep_count + 1) * sizeof(Cell*));
    curr_cell->dependencies[curr_cell->dep_count++] = dep_cell;
    
    dep_cell->dependents = (Cell**)realloc(dep_cell->dependents, (dep_cell->depd_count + 1) * sizeof(Cell*));
    dep_cell->dependents[dep_cell->depd_count++] = curr_cell;
}

// Function to remove dependency
void remove_dependency(Cell* curr_cell, Cell* dep_cell) {
    for (int i = 0; i < curr_cell->dep_count; i++) {
        if (curr_cell->dependencies[i] == dep_cell) {
            for (int j = i; j < curr_cell->dep_count - 1; j++) {
                curr_cell->dependencies[j] = curr_cell->dependencies[j + 1];
            }
            curr_cell->dep_count--;
            curr_cell->dependencies = (Cell**)realloc(curr_cell->dependencies, curr_cell->dep_count * sizeof(Cell*));
            break;
        }
    }
}

// Function to update dependencies
void update_dependencies(Cell* curr_cell, Cell** new_deps, int new_dep_count) {
    for (int i = 0; i < new_dep_count; i++) {
        bool found = false;
        for (int j = 0; j < curr_cell->dep_count; j++) {
            if (curr_cell->dependencies[j] == new_deps[i]) {
                found = true;
                break;
            }
        }
        if (!found) {
            add_dependency(curr_cell, new_deps[i]);
        }
    }
    // Remove old dependencies that are no longer present
    for (int i = 0; i < curr_cell->dep_count; i++) {
        bool still_valid = false;
        for (int j = 0; j < new_dep_count; j++) {
            if (curr_cell->dependencies[i] == new_deps[j]) {
                still_valid = true;
                break;
            }
        }
        if (!still_valid) {
            remove_dependency(curr_cell, curr_cell->dependencies[i]);
        }
    }
}

// Function to check for circular dependencies
bool check_circular_dependency(Cell* cell, bool* visited, bool* stack) {
    if (stack[cell->value]) return true;
    if (visited[cell->value]) return false;
    visited[cell->value] = true;
    stack[cell->value] = true;
    for (int i = 0; i < cell->dep_count; i++) {
        if (check_circular_dependency(cell->dependencies[i], visited, stack)) {
            return true;
        }
    }
    stack[cell->value] = false;
    return false;
}

// Function to update dependents using topological sorting
void update_dependents(Cell* curr_cell) {
    // Perform topological sorting and update values in order
    for (int i = 0; i < curr_cell->depd_count; i++) {
        curr_cell->dependents[i]->needs_update = true;
    }
    // Update values of affected cells
    for (int i = 0; i < curr_cell->depd_count; i++) {
        // Evaluate and update values
        curr_cell->dependents[i]->value = curr_cell->dependents[i]->value + 1; // Placeholder for real evaluation
    }
}

int main() {
    // Example usage
    Cell* A1 = create_cell("A1");
    Cell* A2 = create_cell("A2");
    add_dependency(A1, A2);
    update_dependents(A1);
    
    printf("Updated A2 value: %d\n", A2->value);
    
    free(A1);
    free(A2);
    return 0;
}
