#include "../Declarations/ds.h"
#include "../Declarations/parser.h"
#include "../Declarations/backend.h"
#include "../Declarations/frontend.h"

void print_cell(Cell* cell) {printf("%s%d: %d\n", cell->col, cell->row+1, cell->value);
}
void print_dependents(Cell* cell){
    printf("Dependents of %s%d: ", cell->col, cell->row+1);
    if(cell->dependents != NULL){
        SetIterator it;
        set_iterator_init(&it, cell->dependents);
        while (set_iterator_has_next(&it)) {
            Cell* dep = set_iterator_next(&it);
            printf("%s%d ", dep->col, dep->row+1);
        }
        set_iterator_free(&it);
    }
    printf("\n");
}

// Function to update the dependencies of a cell: 1 -> no cycle/updated successfully, 0 -> cycle/not updated
int update_dependencies(Cell* curr_cell, Set* new_dependencies) {
    printf("$ Updating dependencies for cell %s%d\n", curr_cell->col, curr_cell->row+1);
    // Create a copy of the old dependencies
    Set old_dependencies;
    set_init(&old_dependencies);
    if (curr_cell->dependencies != NULL) {
        SetIterator old_it;
        set_iterator_init(&old_it, curr_cell->dependencies);
        while (set_iterator_has_next(&old_it)) {
            Cell* dep = set_iterator_next(&old_it);
            set_add(&old_dependencies, dep);
        }
        set_iterator_free(&old_it);
    }

    // Remove current cell from the dependents set of old dependencies
    SetIterator old_it;
    set_iterator_init(&old_it, &old_dependencies);
    while (set_iterator_has_next(&old_it)) {
        Cell* old_dep = set_iterator_next(&old_it);
        if (old_dep->dependents != NULL) set_remove(old_dep->dependents, curr_cell);
    }
    set_iterator_free(&old_it);

    // Create a copy of the new dependencies
    Set new_deps_copy;
    set_init(&new_deps_copy);
    SetIterator new_it;
    set_iterator_init(&new_it, new_dependencies);
    while (set_iterator_has_next(&new_it)) {
        Cell* new_dep = set_iterator_next(&new_it);
        set_add(&new_deps_copy, new_dep);
    }
    set_iterator_free(&new_it);

    // Add current cell to the dependents set of new dependencies
    set_iterator_init(&new_it, &new_deps_copy);
    printf("$ New dependencies: ");
    while (set_iterator_has_next(&new_it)) {
        Cell* new_dep = set_iterator_next(&new_it);
        print_cell(new_dep);
        if (new_dep->dependents == NULL) {
            new_dep->dependents = (Set*)malloc(sizeof(Set));
            set_init(new_dep->dependents);
        }
        set_add(new_dep->dependents, curr_cell);
        print_dependents(new_dep);
    }
    set_iterator_free(&new_it);
    printf("\n");


    // change the depencies of the current cell
    if (curr_cell->dependencies != NULL) {
        set_free(curr_cell->dependencies);
        free(curr_cell->dependencies);
    }
    curr_cell->dependencies = (Set*)malloc(sizeof(Set));
    set_init(curr_cell->dependencies);
    set_iterator_init(&new_it, &new_deps_copy);
    while (set_iterator_has_next(&new_it)) {
        Cell* new_dep = set_iterator_next(&new_it);
        set_add(curr_cell->dependencies, new_dep);
    }
    set_iterator_free(&new_it);

    // Check for circular dependencies
    if (check_circular_dependencies(curr_cell)) {
        // Circular dependency detected, revert changes
        printf("Circular dependency detected. Reverting changes.\n");

        // Remove current cell from the dependents set of new dependencies
        set_iterator_init(&new_it, &new_deps_copy);
        while (set_iterator_has_next(&new_it)) {
            Cell* new_dep = set_iterator_next(&new_it);
            if (new_dep->dependents != NULL) set_remove(new_dep->dependents, curr_cell);
        }
        set_iterator_free(&new_it);
        
        // Re-add current cell to the dependents set of old dependencies
        set_iterator_init(&old_it, &old_dependencies);
        while (set_iterator_has_next(&old_it)) {
            Cell* old_dep = set_iterator_next(&old_it);
            if (old_dep->dependents == NULL) {
                old_dep->dependents = (Set*)malloc(sizeof(Set));
                set_init(old_dep->dependents);
            }
            set_add(old_dep->dependents, curr_cell);
        }
        set_iterator_free(&old_it);

        // Revert the dependencies of the current cell
        if (curr_cell->dependencies != NULL) {
            set_free(curr_cell->dependencies);
            free(curr_cell->dependencies);
        }
        curr_cell->dependencies = (Set*)malloc(sizeof(Set));
        set_init(curr_cell->dependencies);
        set_iterator_init(&old_it, &old_dependencies);
        while (set_iterator_has_next(&old_it)) {
            Cell* old_dep = set_iterator_next(&old_it);
            set_add(curr_cell->dependencies, old_dep);
        }
        set_iterator_free(&old_it);

        // Free the temporary sets
        set_free(&old_dependencies);
        set_free(&new_deps_copy);

        return 0;
    }

    // Free the temporary sets
    set_free(&old_dependencies);
    set_free(&new_deps_copy);

    // Call the function to update the cell's value based on its new dependencies
    return 1;
}


// Helper function for circular dependency check
bool detect_cycle_dfs(Cell* curr_cell, Set* visited, Set* recursion_stack) {
    // Mark current cell as visited and add to recursion stack
    set_add(visited, curr_cell);
    set_add(recursion_stack, curr_cell);

    // Visit all dependencies
    if (curr_cell->dependencies != NULL) {
        SetIterator it;
        set_iterator_init(&it, curr_cell->dependencies);
        while (set_iterator_has_next(&it)) {
            Cell* dep = set_iterator_next(&it);
            
            // If not visited, recurse
            if (set_find(visited, dep) == NULL) {
                if (detect_cycle_dfs(dep, visited, recursion_stack)) {
                    set_iterator_free(&it);
                    return true;
                }
            }
            // If already in recursion stack, we found a cycle
            else if (set_find(recursion_stack, dep) != NULL) {
                set_iterator_free(&it);
                return true;
            }
        }
        set_iterator_free(&it);
    }

    // Remove from recursion stack and return
    set_remove(recursion_stack, curr_cell);
    return false;
}

bool check_circular_dependencies(Cell* curr_cell) {
    Set visited, recursion_stack;
    set_init(&visited);
    set_init(&recursion_stack);

    bool has_cycle = detect_cycle_dfs(curr_cell, &visited, &recursion_stack);

    set_free(&visited);
    set_free(&recursion_stack);
    return has_cycle;
}

// Helper function to collect all dependent cells for topological sort
void collect_dependents(Cell* curr_cell, Set* affected_cells) {
    if (curr_cell->dependents != NULL) {
        SetIterator it;
        set_iterator_init(&it, curr_cell->dependents);
        while (set_iterator_has_next(&it)) {
            Cell* dep = set_iterator_next(&it);
            if (set_find(affected_cells, dep) == NULL) {
                set_add(affected_cells, dep);
                collect_dependents(dep, affected_cells);
            }
        }
        set_iterator_free(&it);
    }
}

void update_dependents(Cell* curr_cell) {
    if(curr_cell->dependents == NULL) return;
    // Collect all affected cells
    Set affected_cells;
    set_init(&affected_cells);
    collect_dependents(curr_cell, &affected_cells);

    // Create cell mapping and adjacency matrix for topological sort
    int num_cells = 0;
    SetIterator count_it;

    set_iterator_init(&count_it, &affected_cells);
    while (set_iterator_has_next(&count_it)) {
        num_cells++;
        print_cell(set_iterator_next(&count_it));
        // set_iterator_next(&count_it);
    }
    set_iterator_free(&count_it);

    printf("$ Number of affected cells: %d\n", num_cells);
    if (num_cells == 0) {
        set_free(&affected_cells); return;
    }
    // Create cell mapping
    Cell** cell_map = (Cell**)malloc((num_cells + 1) * sizeof(Cell*));
    if (cell_map == NULL) {
        fprintf(stderr, "Memory allocation failed for cell_map\n");
        set_free(&affected_cells);
        return;
    }
    int index = 1;
    SetIterator map_it;
    set_iterator_init(&map_it, &affected_cells);
    while (set_iterator_has_next(&map_it)) {
        Cell* cell = set_iterator_next(&map_it);
        cell->topo_order = index;
        cell_map[index++] = cell;
    }
    set_iterator_free(&map_it);
    // Create adjacency matrix
    Set* adj_list = (Set*)malloc((num_cells + 1) * sizeof(Set));
    if(adj_list == NULL) {
        fprintf(stderr, "Memory allocation failed for adj_list\n");
        free(cell_map);
        set_free(&affected_cells);
        return;
    }

    for (int i = 1; i <= num_cells; i++) set_init(&adj_list[i]);
    
    // Fill adjacency list - only add edges between affected cells
    for (int i = 1; i <= num_cells; i++) {
        Cell* cell = cell_map[i];
        if (cell->dependencies != NULL) {
            SetIterator dep_it;
            set_iterator_init(&dep_it, cell->dependencies);
            while (set_iterator_has_next(&dep_it)) {
                Cell* dep = set_iterator_next(&dep_it);
                // Only add edge if dependency is in affected_cells
                if (set_find(&affected_cells, dep) != NULL) set_add(&adj_list[i], dep);
            }
            set_iterator_free(&dep_it);
        }
    }

    // Perform topological sort
    Vector sorted = topological_sort(adj_list, num_cells, cell_map);

    // Update cells in topological order
    VectorIterator update_it;
    vector_iterator_init(&update_it, &sorted);
    printf("$ TopoSorted: ");
    while (vector_iterator_has_next(&update_it)) {
        Cell* cell = vector_iterator_next(&update_it);
        printf("%s%d ", cell->col, cell->row+1);
    }
    printf("\n");

    bool divbyzeroflag = (evaluate_cell(curr_cell) == -1);
    vector_iterator_init(&update_it, &sorted);
    while (vector_iterator_has_next(&update_it)) {
        Cell* cell = vector_iterator_next(&update_it);
        // Recalculate cell value
        // Note: This is where you'd call your cell evaluation function
        // For now, we'll just check for division by zero
        if(divbyzeroflag) cell->has_error = true;
        else evaluate_cell(cell);
    }
    // Cleanup
    vector_free(&sorted);
    for (int i = 1; i <= num_cells; i++) set_free(&adj_list[i]);
    free(adj_list);
    free(cell_map);
    set_free(&affected_cells);  
}

Set* createDependenciesSet(Vector* List){
    Set* dependencies = (Set*)malloc(sizeof(Set));
    set_init(dependencies);
    VectorIterator it;
    vector_iterator_init(&it, List);
    while (vector_iterator_has_next(&it)) {
        Cell* cell = vector_iterator_next(&it);
        set_add(dependencies, cell); // vector mein jo copied cell hai uska pointer hai set mein
    }
    return dependencies;
}

void editCell(Spreadsheet *sheet)
{
    printf("Enter command: ");
    restore_terminal();

    char input_line[MAX_CELL_LENGTH];
    fgets(input_line, MAX_CELL_LENGTH, stdin);
    input_line[strcspn(input_line, "\n")] = 0;
    if (strlen(input_line) > 0) process_command(sheet, input_line);
    configure_terminal();
}


int evaluate_cell(Cell *cell)
{
    if (cell -> has_error) return -1;
    switch(cell->type)
    {
        case TYPE_EMPTY:
            return 0;
            break;
        
        case TYPE_CONSTANT:
            if (cell->is_sleep && cell->value>0) sleep(cell->value);
            return 0;
            break;

        case TYPE_ARITHMETIC:
        {
            int left, right;
            left = (cell->op_data.arithmetic.operand1 != NULL) ? 
                cell->op_data.arithmetic.operand1->value : cell->op_data.arithmetic.constant;

            right = (cell->op_data.arithmetic.operand2 != NULL) ? 
                cell->op_data.arithmetic.operand2->value : cell->op_data.arithmetic.constant;

            switch(cell->op_data.arithmetic.op) 
            {
                case OP_ADD:
                    cell->value = left + right; return 0;
                case OP_SUB:
                    cell->value = left - right; return 0;
                case OP_MUL:
                    cell->value = left * right; return 0;
                case OP_DIV:
                    if (right == 0) {cell->has_error = true; return -1;}
                    cell->value = left / right;
                    return 0;
                default:
                    cell->has_error = true; return -1;
            }
            break;
        }    
        case TYPE_FUNCTION:
            if ((cell->op_data.function.range_size)<=0) {cell->has_error=true;return -1;}

            int sum = 0, count = 0;
            int min_val = INT_MAX, max_val = INT_MIN;
            double sum_sq = 0.0;

            SetIterator it;
            set_iterator_init(&it, cell->dependencies);
            Cell *dep;
            while ((dep = set_iterator_next(&it)) != NULL)
            {
                int dep_val = dep->value;
                if (dep->has_error)
                {
                    cell->has_error = true;
                    set_iterator_free(&it);
                    return -1;
                }
                sum += dep_val;
                if (dep_val < min_val) min_val = dep_val;
                if (dep_val > max_val) max_val = dep_val;
                sum_sq += dep_val * dep_val;
                count++;
            }
            set_iterator_free(&it);
            if (count == 0)
            {
                cell->has_error = true; return -1;
            }
            if (strcmp(cell->op_data.function.func_name, "SUM") == 0) {cell->value = sum;}
            else if (strcmp(cell->op_data.function.func_name, "AVG") == 0) {cell->value = sum / count;}
            else if (strcmp(cell->op_data.function.func_name, "MIN") == 0) {cell->value = min_val;}
            else if (strcmp(cell->op_data.function.func_name, "MAX") == 0) {cell->value = max_val;}
            else if (strcmp(cell->op_data.function.func_name, "STDEV") == 0)
            {
                double mean = (double)sum / count;
                double variance = (sum_sq / count) - (mean * mean);
                cell->value = (int)sqrt(variance);
            }
            else {cell->has_error = true; return -1;}
            break;
        case TYPE_REFERENCE:
            cell->value = cell->op_data.ref->value;
            if (cell->is_sleep && cell->value >0) sleep(cell->value);
            break;
    }
    return 0;
}