#include "../Declarations/ds.h"
#include "../Declarations/parser.h"
#include "../Declarations/backend.h"
#include "../Declarations/frontend.h"
// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./bin/sheet 10 10


void print_cell(Cell* cell) {
    printf("%d%d: %d\n", cell->col, cell->row+1, cell->value);
}
void print_dependents(Cell* cell){
    // printf("Dependents of %s%d: ", cell->col, cell->row+1);
    if(cell->dependents != NULL){
        SetIterator it;
        set_iterator_init(&it, cell->dependents);
        while (set_iterator_has_next(&it)) {
            Cell* dep = set_iterator_next(&it);
            printf("%d%d ", dep->col, dep->row+1);
        }
        set_iterator_free(&it);
    }
    // printf("\n");
}



// Function to update the dependencies of a cell: 1 -> no cycle/updated successfully, 0 -> cycle/not updated
int update_dependencies(Cell* curr_cell, Set* new_dependencies, Spreadsheet* sheet) {
    // printf("$ Updating dependencies for cell %s%d\n", curr_cell->col, curr_cell->row+1);
    // Create a copy of the old dependencies
    if(set_find(new_dependencies, curr_cell->row, curr_cell->col) != NULL){
        // printf("Circular dependency detected. Reverting changes.\n");
        set_free(new_dependencies);
        free(new_dependencies);
        new_dependencies = NULL;
        return 0;
    }

    //(OPTIMISATION) removing copy to make it faster
    Set *old_deps = curr_cell->dependencies; // this points to this memory block so we don't lose it
    curr_cell->dependencies = new_dependencies; // this points to the new memory block

    // Remove current cell from the dependents set of old dependencies
    if(old_deps != NULL){
        if(curr_cell->dependencies->type == 'F'){
            SetIterator old_it;
            set_iterator_init(&old_it, curr_cell->dependencies);
            Cell *temp = NULL, *dep2 = NULL, *dep3 = NULL;
            bool cnt = 0;
            while ((temp = set_iterator_next(&old_it)) != NULL)
            {
                if(!cnt){
                    dep2 = temp;
                    cnt = 1;
                }else{
                    dep3 = temp;
                    break;
                }
            }
            set_iterator_free(&old_it);

            short r1 = dep2->row, c1 = dep2->col;
            short r2 = dep3->row, c2 = dep3->col;

            for (short i = r1; i <= r2; i++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell* old_dep = &sheet->cells[i][j];
                    if (old_dep->dependents != NULL) set_remove(old_dep->dependents, curr_cell->row, curr_cell->col);
                }
            }
        }else{
            SetIterator old_it;
            set_iterator_init(&old_it, old_deps);
            while (set_iterator_has_next(&old_it)) {
                Cell* old_dep = set_iterator_next(&old_it);
                if (old_dep->dependents != NULL) set_remove(old_dep->dependents, curr_cell->row, curr_cell->col);
            }
            set_iterator_free(&old_it);
        }

    }


    // Add current cell to the dependents set of new dependencies
    if(new_dependencies != NULL){
        if(new_dependencies->type == 'F'){
            SetIterator new_it;
            set_iterator_init(&new_it, new_dependencies);
            Cell *temp = NULL, *dep2 = NULL, *dep3 = NULL;
            bool cnt = 0;
            while ((temp = set_iterator_next(&new_it)) != NULL)
            {
                if(!cnt){
                    dep2 = temp;
                    cnt = 1;
                }else{
                    dep3 = temp;
                    break;
                }
            }
            set_iterator_free(&new_it);

            short r1 = dep2->row, c1 = dep2->col;
            short r2 = dep3->row, c2 = dep3->col;

            for (short i = r1; i <= r2; i++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell* new_dep = &sheet->cells[i][j];
                    if (new_dep->dependents == NULL) {
                        new_dep->dependents = (Set*)malloc(sizeof(Set));
                        set_init(new_dep->dependents, sheet);
                    }
                    set_add(new_dep->dependents, curr_cell->row, curr_cell->col);
                }
            }
        }else{
            SetIterator new_it;
            set_iterator_init(&new_it, new_dependencies);
            // printf("$ New dependencies: ");
            while (set_iterator_has_next(&new_it)) {
                Cell* new_dep = set_iterator_next(&new_it);
                // print_cell(new_dep);
                if (new_dep->dependents == NULL) {
                    new_dep->dependents = (Set*)malloc(sizeof(Set));
                    set_init(new_dep->dependents, sheet);
                }
                set_add(new_dep->dependents, curr_cell->row, curr_cell->col);
                // print_dependents(new_dep);
            }
            set_iterator_free(&new_it);
        }
    }
    // printf("\n");

    // Check for circular dependencies
    if (check_circular_dependencies(curr_cell, sheet)) {
        // Circular dependency detected, revert changes
        // printf("Circular dependency detected. Reverting changes.\n");

        // Remove current cell from the dependents set of new dependencies
        if(new_dependencies != NULL){

            if(new_dependencies->type == 'F'){

                SetIterator new_it;
                set_iterator_init(&new_it, new_dependencies);
                Cell *temp = NULL, *dep2 = NULL, *dep3 = NULL;
                bool cnt = 0;
                while ((temp = set_iterator_next(&new_it)) != NULL)
                {
                    if(!cnt){
                        dep2 = temp;
                        cnt = 1;
                    }else{
                        dep3 = temp;
                        break;
                    }
                }
                set_iterator_free(&new_it);

                short r1 = dep2->row, c1 = dep2->col;
                short r2 = dep3->row, c2 = dep3->col;

                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell* new_dep = &sheet->cells[i][j];
                        if (new_dep->dependents != NULL) set_remove(new_dep->dependents, curr_cell->row, curr_cell->col);
                    }
                }

            }else{

                SetIterator new_it;
                set_iterator_init(&new_it, new_dependencies);
                while (set_iterator_has_next(&new_it)) {
                    Cell* new_dep = set_iterator_next(&new_it);
                    if (new_dep->dependents != NULL) set_remove(new_dep->dependents, curr_cell->row, curr_cell->col);
                }
                set_iterator_free(&new_it);

            }

        }
        // Re-add current cell to the dependents set of old dependencies
        if(old_deps != NULL){

            if(old_deps->type == 'F'){
                SetIterator old_it;
                set_iterator_init(&old_it, curr_cell->dependencies);
                Cell *temp = NULL, *dep2 = NULL, *dep3 = NULL;
                bool cnt = 0;
                while ((temp = set_iterator_next(&old_it)) != NULL)
                {
                    if(!cnt){
                        dep2 = temp;
                        cnt = 1;
                    }else{
                        dep3 = temp;
                        break;
                    }
                }
                set_iterator_free(&old_it);

                short r1 = dep2->row, c1 = dep2->col;
                short r2 = dep3->row, c2 = dep3->col;

                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell* old_dep = &sheet->cells[i][j];
                        if (old_dep->dependents == NULL) {
                            old_dep->dependents = (Set*)malloc(sizeof(Set));
                            set_init(old_dep->dependents, sheet);
                        }
                        set_add(old_dep->dependents, curr_cell->row, curr_cell->col);
                    }
                }
            }else{
                SetIterator old_it;
                set_iterator_init(&old_it, old_deps);
                while (set_iterator_has_next(&old_it)) {
                    Cell* old_dep = set_iterator_next(&old_it);
                    if (old_dep->dependents == NULL) {
                        old_dep->dependents = (Set*)malloc(sizeof(Set));
                        set_init(old_dep->dependents, sheet);
                    }
                    set_add(old_dep->dependents, curr_cell->row, curr_cell->col);
                }
                set_iterator_free(&old_it);
            }

        }


        curr_cell->dependencies = old_deps;

        set_free(new_dependencies);
        free(new_dependencies);
        new_dependencies = NULL;
        return 0;
    }

    // free(old_deps);
    set_free(old_deps);
    free(old_deps);
    old_deps = NULL;
    // Call the function to update the cell's value based on its new dependencies
    return 1;
}

// Helper function for circular dependency check
bool detect_cycle_dfs(Cell* curr_cell, Set* visited, Set* recursion_stack, Spreadsheet* sheet) {
    // Mark current cell as visited and add to recursion stack
    set_add(visited, curr_cell->row, curr_cell->col);
    set_add(recursion_stack, curr_cell->row, curr_cell->col);

    // Visit all dependencies
    if (curr_cell->dependencies != NULL) {

        if(curr_cell->dependencies->type == 'F'){
            SetIterator new_it;
            set_iterator_init(&new_it, curr_cell->dependencies);
            Cell *temp = NULL, *dep2 = NULL, *dep3 = NULL;
            bool cnt = 0;
            while ((temp = set_iterator_next(&new_it)) != NULL)
            {
                if(!cnt){
                    dep2 = temp;
                    cnt = 1;
                }else{
                    dep3 = temp;
                    break;
                }
            }
            set_iterator_free(&new_it);

            short r1 = dep2->row, c1 = dep2->col;
            short r2 = dep3->row, c2 = dep3->col;

            for (short i = r1; i <= r2; i++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell * dep = &sheet->cells[i][j];
                    
                    if (set_find(visited, dep->row, dep->col) == NULL) {
                        if (detect_cycle_dfs(dep, visited, recursion_stack, sheet)) {
                            return true;
                        }
                    }
                    // If already in recursion stack, we found a cycle
                    else if (set_find(recursion_stack, dep->row, dep->col) != NULL) {
                        return true;
                    }                    
                }
            }
        }else{
            SetIterator it;
            set_iterator_init(&it, curr_cell->dependencies);
            while (set_iterator_has_next(&it)) {
                Cell* dep = set_iterator_next(&it);
                
                // If not visited, recurse
                if (set_find(visited, dep->row, dep->col) == NULL) {
                    if (detect_cycle_dfs(dep, visited, recursion_stack, sheet)) {
                        set_iterator_free(&it);
                        return true;
                    }
                }
                // If already in recursion stack, we found a cycle
                else if (set_find(recursion_stack, dep->row, dep->col) != NULL) {
                    set_iterator_free(&it);
                    return true;
                }
            }
            set_iterator_free(&it);
        }
    }

    // Remove from recursion stack and return
    set_remove(recursion_stack, curr_cell->row, curr_cell->col);
    return false;
}

bool check_circular_dependencies(Cell* curr_cell, Spreadsheet* sheet) {
    Set visited, recursion_stack;
    set_init(&visited, sheet);
    set_init(&recursion_stack, sheet);

    bool has_cycle = detect_cycle_dfs(curr_cell, &visited, &recursion_stack, sheet);

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
            if (set_find(affected_cells, dep->row, dep->col) == NULL) {
                set_add(affected_cells, dep->row, dep->col);
                collect_dependents(dep, affected_cells);
            }
        }
        set_iterator_free(&it);
    }
}




void update_dependents(Cell* curr_cell, Spreadsheet* sheet) {
    if(curr_cell->dependents == NULL) return;
    // Collect all affected cells
    Set affected_cells;
    set_init(&affected_cells, sheet);
    collect_dependents(curr_cell, &affected_cells);

    // Create cell mapping and adjacency matrix for topological sort
    int num_cells = 0;
    SetIterator count_it;

    set_iterator_init(&count_it, &affected_cells);
    while (set_iterator_has_next(&count_it)) {
        num_cells++;
        //print_cell(set_iterator_next(&count_it));
        set_iterator_next(&count_it);
    }
    set_iterator_free(&count_it);

    // printf("$ Number of affected cells: %d\n", num_cells);
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
        cell_map = NULL;
        set_free(&affected_cells);
        return;
    }

    for (int i = 1; i <= num_cells; i++) set_init(&adj_list[i], sheet);
    
    // Fill adjacency list - only add edges between affected cells
    for (int i = 1; i <= num_cells; i++) {
        Cell* cell = cell_map[i];
        if (cell->dependencies != NULL) {
            if(cell->dependencies->type == 'F'){
                SetIterator dep_it;
                set_iterator_init(&dep_it, cell->dependencies);
                Cell *dep2 = NULL, *dep3 = NULL;
                Cell *temp; bool cnt = 0;
                while ((temp = set_iterator_next(&dep_it)) != NULL)
                {
                    if(!cnt){
                        dep2 = temp;
                        cnt = 1;
                    }else{
                        dep3 = temp;
                        break;
                    }
                }
                set_iterator_free(&dep_it);

                short r1 = dep2->row, c1 = dep2->col;
                short r2 = dep3->row, c2 = dep3->col;

                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell* dep = &sheet->cells[i][j];
                        // Only add edge if dependency is in affected_cells.
                        if (set_find(&affected_cells, dep->row, dep->col) != NULL) set_add(&adj_list[i], dep->row, dep->col);
                    }
                }

            }else{
                SetIterator dep_it;
                set_iterator_init(&dep_it, cell->dependencies);
                while (set_iterator_has_next(&dep_it)) {
                    Cell* dep = set_iterator_next(&dep_it);
                    // Only add edge if dependency is in affected_cells.
                    if (set_find(&affected_cells, dep->row, dep->col) != NULL) set_add(&adj_list[i], dep->row, dep->col);
                }
                set_iterator_free(&dep_it);
            }

        }
    }

    // Perform topological sort
    Vector sorted;
    // vector_init(&sorted, sheet);
    topological_sort(adj_list, num_cells, cell_map, &sorted, sheet);

    // Update cells in topological order
    VectorIterator update_it;
    vector_iterator_init(&update_it, &sorted);
    // printf("$ TopoSorted: ");
    while (vector_iterator_has_next(&update_it)) {
        Cell* cell = vector_iterator_next(&update_it);
        printf("%d%d ", cell->col, cell->row+1);
    }
    // printf("\n");

    bool divbyzeroflag = (evaluate_cell(curr_cell, sheet) == -1);
    vector_iterator_init(&update_it, &sorted);
    while (vector_iterator_has_next(&update_it)) {
        Cell* cell = vector_iterator_next(&update_it);
        // Recalculate cell value
        // Note: This is where you'd call your cell evaluation function
        // For now, we'll just check for division by zero
        if(divbyzeroflag) cell->has_error = true;
        else evaluate_cell(cell, sheet);
    }
    // Cleanup
    vector_free(&sorted);
    for (int i = 1; i <= num_cells; i++) set_free(&adj_list[i]);
    free(adj_list);
    free(cell_map);
    adj_list = NULL;
    cell_map = NULL;
    set_free(&affected_cells); 
}


void editCell(Spreadsheet *sheet)
{
    printf("Enter command: ");
    restore_terminal();

    char input_line[MAX_CELL_LENGTH];
    if (fgets(input_line, MAX_CELL_LENGTH, stdin) != NULL) {
        input_line[strcspn(input_line, "\n")] = 0;
        if (strlen(input_line) > 0) process_command(sheet, input_line);
    }
    configure_terminal();
}

int evaluate_cell(Cell *cell, Spreadsheet *sheet)
{
    if (cell -> has_error) return -1;
    switch(cell->type)
    {        
        case 'C':
            if (cell->is_sleep && cell->value>0) sleep(cell->value);
            return 0;
            break;

        case 'A':
        {
            int left, right;
            if(cell->op_data.arithmetic.operand1.i != SHRT_MAX){
                left = sheet->cells[cell->op_data.arithmetic.operand1.i][cell->op_data.arithmetic.operand1.j].value;
            }else{
                left = cell->op_data.arithmetic.constant;
            }
            if(cell->op_data.arithmetic.operand2.i != SHRT_MAX){
                right = sheet->cells[cell->op_data.arithmetic.operand2.i][cell->op_data.arithmetic.operand2.j].value;
            }else{
                right = cell->op_data.arithmetic.constant;
            }

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
        case 'F':
            if ((cell->op_data.function.range_size)<=0) {cell->has_error=true;return -1;}

            int sum = 0, count = 0;
            int min_val = INT_MAX, max_val = INT_MIN;
            double sum_sq = 0.0;

            SetIterator it2;
            set_iterator_init(&it2, cell->dependencies);
            Cell *temp = NULL, *dep2 = NULL, *dep3 = NULL;
            bool cnt = 0;
            while ((temp = set_iterator_next(&it2)) != NULL)
            {
                if(!cnt){
                    dep2 = temp;
                    cnt = 1;
                }else{
                    dep3 = temp;
                    break;
                }
            }
            set_iterator_free(&it2);

            short r1 = dep2->row, c1 = dep2->col;
            short r2 = dep3->row, c2 = dep3->col;

            for (short i = r1; i <= r2; i++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    int dep_val = sheet->cells[i][j].value;
                    if (sheet->cells[i][j].has_error)
                    {
                        cell->has_error = true;
                        return -1;
                    }
                    sum += dep_val;
                    if (dep_val < min_val) min_val = dep_val;
                    if (dep_val > max_val) max_val = dep_val;
                    sum_sq += dep_val * dep_val;
                    count++;
                }
            }
            
            
            if (count == 0)
            {
                cell->has_error = true; return -1;
            }
            if (cell->op_data.function.func_name == 'D') {cell->value = sum;}
            else if (cell->op_data.function.func_name == 'C') {cell->value = sum / count;}
            else if (cell->op_data.function.func_name == 'A') {cell->value = min_val;}
            else if (cell->op_data.function.func_name == 'B') {cell->value = max_val;}
            else if (cell->op_data.function.func_name == 'E')
            {
                double mean = (double)sum / count;
                double variance = (sum_sq / count) - (mean * mean);
                cell->value = (int)sqrt(variance);
            }
            else {cell->has_error = true; return -1;}
            break;
        case 'R':
            cell->value = cell->op_data.ref->value;
            if (cell->is_sleep && cell->value >0) sleep(cell->value);
            break;
    }
    return 0;
}