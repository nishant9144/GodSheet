#include "../Declarations/ds.h"
#include "../Declarations/parser.h"
#include "../Declarations/backend.h"
#include "../Declarations/frontend.h"
// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./bin/sheet 10 10
// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --trace-malloc=yes --trace-children=yes --tool=memcheck --log-file=valgrind-out.txt ./bin/sheet 10 10
// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --keep-stacktraces=alloc-and-free --verbose --trace-malloc=yes --trace-children=yes --tool=memcheck --log-file=valgrind-out.txt ./bin/sheet 10 10

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

    short r1 = cellcopy.dependencies.first.i, c1 = cellcopy.dependencies.first.j;
    short r2 = cellcopy.dependencies.second.i, c2 = cellcopy.dependencies.second.j;

    if (cellcopy.type == 'F')
    {
        for (short i = r1; i <= r2; i++)
        {
            for (short j = c1; j <= c2; j++)
            {
                Cell *old_dep = &sheet->cells[i][j];
                old_dep->dependents = avl_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
            }
        }
    }
    else if (cellcopy.type == 'A')
    {
        if (r1 != -1 && c1 != -1)
        {  
            Cell *old_dep = &sheet->cells[r1][c1];
            old_dep->dependents = avl_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
        }
        if (r2 != -1 && c2 != -1)
        {
            Cell *old_dep = &sheet->cells[r2][c2];
            old_dep->dependents = avl_remove(old_dep->dependents, cellcopy.row, cellcopy.col);
        }

    }

    // Add current cell to the dependents set of new dependencies
    if (need_new_deps)
    {
        r1 = new_pairs->first.i, c1 = new_pairs->first.j;
        r2 = new_pairs->second.i, c2 = new_pairs->second.j;

        if (curr_cell->type == 'F')
        {
            for (short i = r1; i <= r2; i++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell *new_dep = &sheet->cells[i][j];
                    new_dep->dependents = avl_insert(new_dep->dependents, curr_cell->row, curr_cell->col);
                }
            }
        }
        else if(curr_cell->type == 'A' || curr_cell->type == 'R')
        {
            if (r1 != -1 && c1 != -1)
            {  
                Cell *new_dep = &sheet->cells[r1][c1];
                new_dep->dependents = avl_insert(new_dep->dependents, curr_cell->row, curr_cell->col);
            }
            if (r2 != -1 && c2 != -1)
            {
                Cell *new_dep = &sheet->cells[r2][c2];
                new_dep->dependents = avl_insert(new_dep->dependents, curr_cell->row, curr_cell->col);
            }
        }
    
        // Check for circular dependencies
        if (check_circular_dependencies(curr_cell, sheet))
        {
            // Remove current cell from the dependents set of new dependencies
            r1 = curr_cell->dependencies.first.i, c1 = curr_cell->dependencies.first.j;
            r2 = curr_cell->dependencies.second.i, c2 = curr_cell->dependencies.second.j;

            if (curr_cell->type == 'F')
            {
                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell *new_dep = &sheet->cells[i][j];
                        if (new_dep->dependents != NULL)
                            new_dep->dependents = avl_remove(new_dep->dependents, curr_cell->row, curr_cell->col);
                    }
                }
            }
            else if (curr_cell->type == 'A' || curr_cell->type == 'R')
            {
                if (r1 != -1 && c1 != -1)
                {  
                    Cell *new_deps = &sheet->cells[r1][c1];
                    new_deps->dependents = avl_remove(new_deps->dependents, curr_cell->row, curr_cell->col);
                }
                if (r2 != -1 && c2 != -1)
                {
                    Cell *new_deps = &sheet->cells[r2][c2];
                    new_deps->dependents = avl_remove(new_deps->dependents, curr_cell->row, curr_cell->col);
                }
            }

            // Add to dependents of the old dependencies
            r1 = cellcopy.dependencies.first.i, c1 = cellcopy.dependencies.first.j;
            r2 = cellcopy.dependencies.second.i, c2 = cellcopy.dependencies.second.j;

            if (cellcopy.type == 'F')
            {
                for (short i = r1; i <= r2; i++)
                {
                    for (short j = c1; j <= c2; j++)
                    {
                        Cell *old_dep = &sheet->cells[i][j];
                        old_dep->dependents = avl_insert(old_dep->dependents, curr_cell->row, curr_cell->col);
                    }
                }
            }
            else if (cellcopy.type == 'A' || cellcopy.type == 'R')
            {
                if (r1 != -1 && c1 != -1)
                {  
                    Cell *old_deps = &sheet->cells[r1][c1];
                    old_deps->dependents = avl_insert(old_deps->dependents, curr_cell->row, curr_cell->col);
                }
                if (r2 != -1 && c2 != -1)
                {
                    Cell *old_deps = &sheet->cells[r2][c2];
                    if (old_deps->dependents == NULL)
                    old_deps->dependents = avl_insert(old_deps->dependents, curr_cell->row, curr_cell->col);
                }
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
    
    short r1 = cell->dependencies.first.i, c1 = cell->dependencies.first.j;
    short r2 = cell->dependencies.second.i, c2 = cell->dependencies.second.j;

    if (cell->type == 'F')
    {
        for (short i = r1; i <= r2; i++)
        {
            for (short j = c1; j <= c2; j++)
            {
                Cell *dep = &sheet->cells[i][j];
                vector_push_back(bin, dep->row, dep->col);
                if (detect_cycle_dfs(dep, sheet, bin))
                    return true;
            }
        }
    }
    else if(cell->type == 'A' || cell->type == 'R')
    {
        if (r1 != -1 && c1 != -1)
        {
            Cell *dep = &sheet->cells[r1][c1];
            vector_push_back(bin, dep->row, dep->col);
            if (detect_cycle_dfs(dep, sheet, bin))
                return true;
        }
        if (r2 != -1 && c2 != -1)
        {
            Cell *dep = &sheet->cells[r2][c2];
            vector_push_back(bin, dep->row, dep->col);
            if (detect_cycle_dfs(dep, sheet, bin))
                return true;
        }
    }
    cell->cell_state = 'V';
    return false;
}

void revertChanges(Vector *bins, Spreadsheet * sheet)
{
    if (bins == NULL) return;

    VectorIterator it;
    vector_iterator_init(&it, bins);
    while (vector_iterator_has_next(&it))
    {
        Pair* p = vector_iterator_next(&it);
        sheet->cells[p->i][p->j].cell_state = 'U';
    }
}

bool check_circular_dependencies(Cell *cell, Spreadsheet *sheet)
{
    Vector bin;
    vector_init(&bin);

    vector_push_back(&bin, cell->row, cell->col);

    bool hascycle = detect_cycle_dfs(cell, sheet, &bin);

    revertChanges(&bin, sheet);
    vector_free(&bin);
    return hascycle;
}


static AVLNode* collect_traverse_avl_tree_backend(AVLNode* node, AVLNode** affected_cells, Spreadsheet* sheet, int *num_cells) {
    if (node == NULL) return *affected_cells;
    
    // In-order traversal: left, current, right
    *affected_cells = collect_traverse_avl_tree_backend(node->left, affected_cells, sheet, num_cells);
    
    // Process the current node
    Pair p = node->pair;

    // Only process if not already visited
    if (avl_find(*affected_cells, p.i, p.j) == NULL) {
        *affected_cells = avl_insert(*affected_cells, node->pair.i, node->pair.j);
        (*num_cells)++;
        *affected_cells = collect_traverse_avl_tree_backend((sheet->cells[p.i][p.j]).dependents, affected_cells, sheet, num_cells);
    }
    
    *affected_cells = collect_traverse_avl_tree_backend(node->right, affected_cells, sheet, num_cells);
    return *affected_cells;
}

static void assign_topo_order(AVLNode* affected_cell, Spreadsheet* sheet, Pair** cell_map, int* index) {
    if (affected_cell == NULL) return;
    
    // In-order traversal: left, current, right
    assign_topo_order(affected_cell->left, sheet, cell_map, index);
    
    // Process the current affected_cell
    Pair p = affected_cell->pair;
    sheet->cells[p.i][p.j].topo_order = *index;
    (*cell_map)[*index].i = p.i;
    (*cell_map)[*index].j = p.j;
    (*index)++;
    
    assign_topo_order(affected_cell->right, sheet, cell_map, index);
}


void update_dependents(Cell *curr_cell, Spreadsheet *sheet)
{
    if (curr_cell->dependents == NULL)
        return;

    // Collect all affected cells
    AVLNode *affected_cells = NULL;
    int num_cells = 0;

    affected_cells = collect_traverse_avl_tree_backend(curr_cell->dependents, &affected_cells, sheet, &num_cells);

    // Create cell mapping and adjacency matrix for topological sort
    if (num_cells == 0)
    {
        avl_free(affected_cells);
        affected_cells = NULL;
        return;
    }

    // Create cell mapping
    Pair *cell_map = (Pair *)malloc((num_cells + 1) * sizeof(Pair));

    int index = 1;
    assign_topo_order(affected_cells, sheet, &cell_map, &index);

    // Create adjacency matrix
    Vector *adj_list = (Vector *)malloc((num_cells + 1) * sizeof(Vector));

    for(int i = 1; i <= num_cells; i++)
        vector_init(&adj_list[i]);

    // Fill adjacency list - only add edges between affected cells
    for (int i = 1; i <= num_cells; i++)
    {
        int xx = cell_map[i].i;
        int yy = cell_map[i].j;
        Cell *cell = &sheet->cells[xx][yy];

        short r1 = cell->dependencies.first.i, c1 = cell->dependencies.first.j;
        short r2 = cell->dependencies.second.i, c2 = cell->dependencies.second.j;

        if (cell->type == 'F')
        {
            for (short rr = r1; rr <= r2; rr++)
            {
                for (short j = c1; j <= c2; j++)
                {
                    Cell *dep = &(sheet->cells[rr][j]);
                    // Only add edge if dependency is in affected_cells.
                    if (avl_find(affected_cells, dep->row, dep->col) != NULL)
                        vector_push_back(&adj_list[i], dep->row, dep->col);
                }
            }
        }
        else if (cell->type == 'A' || cell->type == 'R')
        {
            if (r1 != -1 && c1 != -1)
            {
                Cell *dep = &sheet->cells[r1][c1];
                // Only add edge if dependency is in affected_cells.
                if (avl_find(affected_cells, dep->row, dep->col) != NULL)
                    vector_push_back(&adj_list[i], dep->row, dep->col);
            }

            if (r2 != -1 && c2 != -1)
            {
                Cell *dep = &sheet->cells[r2][c2];
                // Only add edge if dependency is in affected_cells.
                if (avl_find(affected_cells, dep->row, dep->col) != NULL)
                    vector_push_back(&adj_list[i], dep->row, dep->col);
            }
        }
    }

    // Perform topological sort
    Vector sorted;
    topological_sort(adj_list, num_cells, &cell_map, &sorted, sheet);

    // Update cells in topological order
    VectorIterator update_it;

    vector_iterator_init(&update_it, &sorted);
    while (vector_iterator_has_next(&update_it))
    {
        Pair* p = vector_iterator_next(&update_it);

        // Recalculate cell value
        Cell *cell = &sheet->cells[p->i][p->j];
        cell->has_error = false;
        cell->topo_order = -1;

        evaluate_cell(cell, sheet);
    }

    // Cleanup
    vector_free(&sorted);
    for (int i = 1; i <= num_cells; i++)
        vector_free(&adj_list[i]);
    
    free(adj_list);
    free(cell_map);
    adj_list = NULL;
    cell_map = NULL;
    avl_free(affected_cells);
    affected_cells = NULL;
}


int evaluate_cell(Cell *cell, Spreadsheet *sheet)
{
    if (cell->has_error)
        return 0;
    switch (cell->type)
    {
    case 'C':
        if (cell->is_sleep && cell->value > 0)
            sleep(cell->value);
        cell->has_error=false;
        break;

    case 'A':
        if(cell->dependencies.first.i != -1){
            if(sheet->cells[cell->dependencies.first.i][cell->dependencies.first.j].has_error){
                cell->has_error = true;
                break;
            }
        }
        if(cell->dependencies.second.i != -1){
            if(sheet->cells[cell->dependencies.second.i][cell->dependencies.second.j].has_error){
                cell->has_error = true;
                break;
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
                return 0;
            }
            cell->value = left / right;
            return 0;
        default:
            return 0;
        }
        cell->has_error=false;
        break;

    case 'F':
        int sum = 0, count = 0;
        int min_val = INT_MAX, max_val = INT_MIN;
        int sum_sq = 0;

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
                    return 0;
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
            if(count <=1){
                cell->value = 0;
            }else{
                int mean = sum / count;
                double variance = (double)((sum_sq) - 2*sum*mean + (mean * mean)*count) / count;
                cell->value = (int)round(sqrt(variance));
            }
            break;
        }
        cell->has_error = false;
        break;

    case 'R':
        short r = cell->dependencies.first.i, c = cell->dependencies.first.j;
        Cell *ref_cell = &sheet->cells[r][c];
        if(ref_cell->has_error){
            cell->has_error = true;
            return 0;
        }
        cell->value = ref_cell->value;
        if (cell->is_sleep && cell->value > 0) sleep(cell->value);
        cell->has_error=false;
        break;
    }
    return 0;
}