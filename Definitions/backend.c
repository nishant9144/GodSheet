#include "../Declarations/ds.h"
#include "../Declarations/parser.h"
#include "../Declarations/backend.h"
#include "../Declarations/frontend.h"
// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./bin/sheet 10 10

void print_cell(Cell *cell)
{
    printf("%d%d: %d\n", cell->col, cell->row + 1, cell->value);
}
void print_dependents(Cell *cell)
{
    // printf("Dependents of %s%d: ", cell->col, cell->row+1);
    if (cell->dependents != NULL)
    {
        SetIterator it;
        set_iterator_init(&it, cell->dependents);
        while (set_iterator_has_next(&it))
        {
            Cell *dep = set_iterator_next(&it);
            printf("%d%d ", dep->col, dep->row + 1);
        }
        set_iterator_free(&it);
    }
}

// Function to update the dependencies of a cell: 1 -> no cycle/updated successfully, 0 -> cycle/not updated
int update_dependencies(Cell *curr_cell, bool need_new_deps, PairOfPair *new_pairs, Spreadsheet *sheet, Cell cellcopy)
{
    if(curr_cell->type == cellcopy.type){
        if(new_pairs->first.i == cellcopy.dependencies.first.i && new_pairs->first.j == cellcopy.dependencies.first.j && new_pairs->second.i == cellcopy.dependencies.second.i && new_pairs->second.j == cellcopy.dependencies.second.j){
            return 1;
        }
    }


    // Remove current cell from the dependents set of old dependencies
    curr_cell->dependencies = *new_pairs;
    if (cellcopy.type == 'F')
    {
        short r1 = cellcopy.dependencies.first.i, c1 = cellcopy.dependencies.first.j;
        short r2 = cellcopy.dependencies.second.i, c2 = cellcopy.dependencies.second.j;

        for (short i = r1; i <= r2; i++)
        {
            for (short j = c1; j <= c2; j++)
            {
                Cell *old_dep = &sheet->cells[i][j];
                if (old_dep->dependents != NULL)
                    set_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
            }
        }
    }
    else if (cellcopy.type == 'A')
    {
        
        short r = cellcopy.dependencies.first.i;
        short c = cellcopy.dependencies.first.j;

        if (r != -1 && c != -1)
        {  
            Cell *old_dep = &sheet->cells[r][c];
            set_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
        }

        r = cellcopy.dependencies.second.i;
        c = cellcopy.dependencies.second.j;

        if (r != -1 && c != -1)
        {
            Cell *old_dep = &sheet->cells[r][c];
            set_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
        }

    }
    else if (cellcopy.type == 'R')
    {
        short r = cellcopy.dependencies.first.i;
        short c = cellcopy.dependencies.first.j;

        Cell *old_dep = &sheet->cells[r][c];
        set_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
    }

    // Add current cell to the dependents set of new dependencies
    if (need_new_deps)
    {
        if (curr_cell->type == 'F')
        {
            short r1 = new_pairs->first.i, c1 = new_pairs->first.j;
            short r2 = new_pairs->second.i, c2 = new_pairs->second.j;

            for (short i = r1; i <= r2; i++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell *new_dep = &sheet->cells[i][j];
                    if (new_dep->dependents == NULL)
                    {
                        new_dep->dependents = (Set *)malloc(sizeof(Set));
                        set_init(new_dep->dependents, sheet);
                    }
                    set_add(new_dep->dependents, curr_cell->row, curr_cell->col);
                }
            }
        }
        else if(curr_cell->type == 'A')
        {
            short r = new_pairs->first.i;
            short c = new_pairs->first.j;

            if (r != -1 && c != -1)
            {  
                Cell *new_dep = &sheet->cells[r][c];
                if (new_dep->dependents == NULL)
                {
                    new_dep->dependents = (Set *)malloc(sizeof(Set));
                    set_init(new_dep->dependents, sheet);
                }
                set_add(new_dep->dependents, curr_cell->row, curr_cell->col);
            }
            r = new_pairs->second.i;
            c = new_pairs->second.j;

            if (r != -1 && c != -1)
            {
                Cell *new_dep = &sheet->cells[r][c];
                if (new_dep->dependents == NULL)
                {
                    new_dep->dependents = (Set *)malloc(sizeof(Set));
                    set_init(new_dep->dependents, sheet);
                }
                set_add(new_dep->dependents, curr_cell->row, curr_cell->col);
            }
        }
        else if (curr_cell->type == 'R')
        {
            short r = new_pairs->first.i;
            short c = new_pairs->first.j;

            Cell *new_dep = &sheet->cells[r][c];
            if (new_dep->dependents == NULL)
            {
                new_dep->dependents = (Set *)malloc(sizeof(Set));
                set_init(new_dep->dependents, sheet);
            }
            set_add(new_dep->dependents, curr_cell->row, curr_cell->col);            
        }
    
        // Check for circular dependencies
        if (check_circular_dependencies(curr_cell, sheet))
        // if (false)
        {
            // Remove current cell from the dependents set of new dependencies
            if (curr_cell->type == 'F')
            {
                short r1 = curr_cell->dependencies.first.i, c1 = curr_cell->dependencies.first.j;
                short r2 = curr_cell->dependencies.second.i, c2 = curr_cell->dependencies.second.j;

                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell *new_dep = &sheet->cells[i][j];
                        if (new_dep->dependents != NULL)
                            set_remove(new_dep->dependents, curr_cell->row, curr_cell->col);
                    }
                }
            }
            else if (curr_cell->type == 'A')
            {
                short r = curr_cell->dependencies.first.i;
                short c = curr_cell->dependencies.first.j;

                if (r != -1 && c != -1)
                {  
                    Cell *new_deps = &sheet->cells[r][c];
                    set_remove(new_deps->dependents, curr_cell->row, curr_cell->col);
                }

                r = curr_cell->dependencies.second.i;
                c = curr_cell->dependencies.second.j;

                if (r != -1 && c != -1)
                {
                    Cell *new_deps = &sheet->cells[r][c];
                    set_remove(new_deps->dependents, curr_cell->row, curr_cell->col);
                }
            }
            else if (curr_cell->type == 'R')
            {
                short r = curr_cell->dependencies.first.i;
                short c = curr_cell->dependencies.first.j;

                Cell *new_deps = &sheet->cells[r][c];
                set_remove(new_deps->dependents, curr_cell->row, curr_cell->col);
            }

            // Add to dependents of the old dependencies
            if (cellcopy.type == 'F')
            {
                short r1 = cellcopy.dependencies.first.i, c1 = cellcopy.dependencies.first.j;
                short r2 = cellcopy.dependencies.second.i, c2 = cellcopy.dependencies.second.j;

                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell *old_dep = &sheet->cells[i][j];
                        if (old_dep->dependents == NULL)
                        {
                            old_dep->dependents = (Set *)malloc(sizeof(Set));
                            set_init(old_dep->dependents, sheet);
                        }
                        set_add(old_dep->dependents, curr_cell->row, curr_cell->col);
                    }
                }
            }
            else if (cellcopy.type == 'A')
            {
                short r = cellcopy.dependencies.first.i;
                short c = cellcopy.dependencies.first.j;

                if (r != -1 && c != -1)
                {  
                    Cell *old_deps = &sheet->cells[r][c];
                    if (old_deps->dependents == NULL)
                    {
                        old_deps->dependents = (Set *)malloc(sizeof(Set));
                        set_init(old_deps->dependents, sheet);
                    }
                    set_add(old_deps->dependents, curr_cell->row, curr_cell->col);
                }

                r = cellcopy.dependencies.second.i;
                c = cellcopy.dependencies.second.j;

                if (r != -1 && c != -1)
                {
                    Cell *old_deps = &sheet->cells[r][c];
                    if (old_deps->dependents == NULL)
                    {
                        old_deps->dependents = (Set *)malloc(sizeof(Set));
                        set_init(old_deps->dependents, sheet);
                    }
                    set_add(old_deps->dependents, curr_cell->row, curr_cell->col);
                }

            }
            else if (cellcopy.type == 'R')
            {
                short r = cellcopy.dependencies.first.i;
                short c = cellcopy.dependencies.first.j;

                Cell *old_deps = &sheet->cells[r][c];
                if (old_deps->dependents == NULL)
                {
                    old_deps->dependents = (Set *)malloc(sizeof(Set));
                    set_init(old_deps->dependents, sheet);
                }
                set_add(old_deps->dependents, curr_cell->row, curr_cell->col);
            }
            return 0;
        }
    }
    return 1;
}

bool detect_cycle_dfs(Cell *cell, Spreadsheet *sheet, Vector *bin)
{
    if (cell->cell_state == 'P')
        return true;
    else if (cell->cell_state == 'V')
        return false;

    cell->cell_state = 'P';

    if (cell->type == 'F')
    {
        short r1 = cell->dependencies.first.i, c1 = cell->dependencies.first.j;
        short r2 = cell->dependencies.second.i, c2 = cell->dependencies.second.j;

        for (short i = r1; i <= r2; i++)
        {
            for (short j = c1; j <= c2; j++)
            {
                Cell *dep = &sheet->cells[i][j];
                vector_push_back(bin, dep->row, dep->col);
                if (detect_cycle_dfs(dep, sheet, bin))
                {
                    return true;
                }
            }
        }
    }
    else if(cell->type == 'A')
    {
        short r = cell->dependencies.first.i;
        short c = cell->dependencies.first.j;

        if (r != -1 && c != -1)
        {
            Cell *dep = &sheet->cells[r][c];
            vector_push_back(bin, dep->row, dep->col);
            if (detect_cycle_dfs(dep, sheet, bin))
            {
                return true;
            }
        }

        r = cell->dependencies.second.i;
        c = cell->dependencies.second.j;

        if (r != -1 && c != -1)
        {
            Cell *dep = &sheet->cells[r][c];
            vector_push_back(bin, dep->row, dep->col);
            if (detect_cycle_dfs(dep, sheet, bin))
            {
                return true;
            }
        }
    }
    else if(cell->type == 'R')
    {
        short r = cell->dependencies.first.i;
        short c = cell->dependencies.first.j;

        Cell *dep = &sheet->cells[r][c];
        vector_push_back(bin, dep->row, dep->col);
        if (detect_cycle_dfs(dep, sheet, bin))
        {
            return true;
        }
    }
    cell->cell_state = 'V';
    return false;
}

void revertChanges(Vector *bins)
{
    if (bins == NULL)
        return;

    VectorIterator it;
    vector_iterator_init(&it, bins);
    while (vector_iterator_has_next(&it))
    {
        Cell *cell = vector_iterator_next(&it);
        cell->cell_state = 'U';
    }
}

bool check_circular_dependencies(Cell *cell, Spreadsheet *sheet)
{
    Vector bin;
    vector_init(&bin, sheet);
    vector_push_back(&bin, cell->row, cell->col);
    bool hascycle = detect_cycle_dfs(cell, sheet, &bin);
    revertChanges(&bin);
    vector_free(&bin);
    return hascycle;
}


// Helper function to collect all dependent cells for topological sort
void collect_dependents(Cell *curr_cell, Set *affected_cells)
{
    if (curr_cell->dependents != NULL)
    {
        SetIterator it;
        set_iterator_init(&it, curr_cell->dependents);
        while (set_iterator_has_next(&it))
        {
            Cell *dep = set_iterator_next(&it);
            if (set_find(affected_cells, dep->row, dep->col) == NULL)
            {
                set_add(affected_cells, dep->row, dep->col);
                collect_dependents(dep, affected_cells);
            }
        }
        set_iterator_free(&it);
    }
}

void update_dependents(Cell *curr_cell, Spreadsheet *sheet)
{
    if (curr_cell->dependents == NULL)
        return;
    // Collect all affected cells
    Set affected_cells;
    set_init(&affected_cells, sheet);
    collect_dependents(curr_cell, &affected_cells);

    // Create cell mapping and adjacency matrix for topological sort
    int num_cells = 0;
    SetIterator count_it;

    set_iterator_init(&count_it, &affected_cells);
    while (set_iterator_has_next(&count_it))
    {
        num_cells++;
        set_iterator_next(&count_it);
    }
    set_iterator_free(&count_it);

    if (num_cells == 0)
    {
        set_free(&affected_cells);
        return;
    }
    // Create cell mapping
    Cell **cell_map = (Cell **)malloc((num_cells + 1) * sizeof(Cell *));
    if (cell_map == NULL)
    {
        fprintf(stderr, "Memory allocation failed for cell_map\n");
        set_free(&affected_cells);
        return;
    }
    int index = 1;
    SetIterator map_it;
    set_iterator_init(&map_it, &affected_cells);
    while (set_iterator_has_next(&map_it))
    {
        Cell *cell = set_iterator_next(&map_it);
        cell->topo_order = index;
        cell_map[index++] = cell;
    }
    set_iterator_free(&map_it);
    // Create adjacency matrix
    Set *adj_list = (Set *)malloc((num_cells + 1) * sizeof(Set));
    if (adj_list == NULL)
    {
        fprintf(stderr, "Memory allocation failed for adj_list\n");
        free(cell_map);
        cell_map = NULL;
        set_free(&affected_cells);
        return;
    }

    for (int i = 1; i <= num_cells; i++)
        set_init(&adj_list[i], sheet);

    // Fill adjacency list - only add edges between affected cells
    for (int i = 1; i <= num_cells; i++)
    {
        Cell *cell = cell_map[i];
        if (cell->type == 'F')
        {
            short r1 = cell->dependencies.first.i, c1 = cell->dependencies.first.j;
            short r2 = cell->dependencies.second.i, c2 = cell->dependencies.second.j;

            for (short rr = r1; rr <= r2; rr++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell *dep = &sheet->cells[rr][j];
                    // Only add edge if dependency is in affected_cells.
                    if (set_find(&affected_cells, dep->row, dep->col) != NULL)
                    set_add(&adj_list[i], dep->row, dep->col);
                }
            }
        }
        else if (cell->type == 'A' || cell->type == 'R')
        {
            short r = cell->dependencies.first.i;
            short c = cell->dependencies.first.j;

            if (r != -1 && c != -1)
            {
            Cell *dep = &sheet->cells[r][c];
            // Only add edge if dependency is in affected_cells.
            if (set_find(&affected_cells, dep->row, dep->col) != NULL)
                set_add(&adj_list[i], dep->row, dep->col);
            }

            r = cell->dependencies.second.i;
            c = cell->dependencies.second.j;

            if (r != -1 && c != -1)
            {
            Cell *dep = &sheet->cells[r][c];
            // Only add edge if dependency is in affected_cells.
            if (set_find(&affected_cells, dep->row, dep->col) != NULL)
                set_add(&adj_list[i], dep->row, dep->col);
            }
        }
    }

    // Perform topological sort
    Vector sorted;
    topological_sort(adj_list, num_cells, cell_map, &sorted, sheet);

    // Update cells in topological order
    VectorIterator update_it;
    vector_iterator_init(&update_it, &sorted);
    // printf("$ TopoSorted: ");
    // while (vector_iterator_has_next(&update_it))
    // {
    //     Cell *cell = vector_iterator_next(&update_it);
    //     printf("%d%d ", cell->col, cell->row + 1);
    // }

    vector_iterator_init(&update_it, &sorted);
    while (vector_iterator_has_next(&update_it))
    {
        Cell *cell = vector_iterator_next(&update_it);
        // Recalculate cell value
        // Note: This is where you'd call your cell evaluation function
        // For now, we'll just check for division by zero
        evaluate_cell(cell, sheet);
    }
    // Cleanup
    vector_free(&sorted);
    for (int i = 1; i <= num_cells; i++)
        set_free(&adj_list[i]);
    free(adj_list);
    free(cell_map);
    adj_list = NULL;
    cell_map = NULL;
    set_free(&affected_cells);
}


int evaluate_cell(Cell *cell, Spreadsheet *sheet)
{
    if (cell->has_error)
        return -1;
    switch (cell->type)
    {
    case 'C':
        if (cell->is_sleep && cell->value > 0)
            sleep(cell->value);
        break;

    case 'A':
        if(cell->dependencies.first.i != -1){
            if(sheet->cells[cell->dependencies.first.i][cell->dependencies.first.j].has_error){
                cell->has_error = true;
                return -1;
            }
        }
        if(cell->dependencies.second.i != -1){
            if(sheet->cells[cell->dependencies.second.i][cell->dependencies.second.j].has_error){
                cell->has_error = true;
                return -1;
            }
        }
        int left, right;
        left = (cell->dependencies.first.i != -1 && cell->dependencies.first.j != -1) ? sheet->cells[cell->dependencies.first.i][cell->dependencies.first.j].value
                                                                                      : cell->op_data.arithmetic.constant;
        right = (cell->dependencies.second.i != -1 && cell->dependencies.second.j != -1) ? sheet->cells[cell->dependencies.second.i][cell->dependencies.second.j].value
                                                                                      : cell->op_data.arithmetic.constant;
                                                                                
        switch (cell->op_data.arithmetic.op)
        {
        case OP_ADD:
            cell->value = left + right;
            return 0;
        case OP_SUB:
            cell->value = left - right;
            return 0;
        case OP_MUL:
            cell->value = left * right;
            return 0;
        case OP_DIV:
            if (right == 0)
            {
                cell->has_error = true;
                sheet->last_status = ERR_DIV_ZERO;
                return -1;
            }
            cell->value = left / right;
            return 0;
        default:
            cell->has_error = true;
            return -1;
        }
        break;

    case 'F':
        int sum = 0, count = 0;
        int min_val = INT_MAX, max_val = INT_MIN;
        double sum_sq = 0.0;

        short r1 = cell->dependencies.first.i, c1 = cell->dependencies.first.j;
        short r2 = cell->dependencies.second.i, c2 = cell->dependencies.second.j;

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
                if (dep_val < min_val)
                    min_val = dep_val;
                if (dep_val > max_val)
                    max_val = dep_val;
                sum_sq += dep_val * dep_val;
                count++;
            }
        }
        switch (cell->op_data.function.func_name)
        {
        case 'D':
            cell->value = sum;
            break;
        case 'C':
            cell->value = sum / count;
            break;
        case 'A':
            cell->value = min_val;
            break;
        case 'B':
            cell->value = max_val;
            break;
        case 'E':
            double mean = (double)sum / count;
            double variance = (sum_sq / count) - (mean * mean);
            cell->value = (int)sqrt(variance);
            break;

        default:
            cell->has_error = true;
            return -1;
        }
        break;

    case 'R':
        short r = cell->dependencies.first.i, c = cell->dependencies.first.j;
        Cell *ref_cell = &sheet->cells[r][c];
        if(ref_cell->has_error){
            cell->has_error = true;
            return -1;
        }
        cell->value = ref_cell->value;
        if (cell->is_sleep && cell->value > 0) sleep(cell->value);
        break;
    }
    return 0;
}