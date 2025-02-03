#include "spreadsheet.h"
#include <stdlib.h>
#include <string.h>

void add_dependency(Spreadsheet *sheet, int src_row, int src_col, int target_row, int target_col)
{
    DependencyNode *node = malloc(sizeof(DependencyNode));
    node->row = target_row;
    node->col = target_col;
    node->next = sheet->cells[src_row][src_col].dependents;
    sheet->cells[src_row][src_col].dependents = node;
}

static bool dfs_cycle_check(Spreadsheet *sheet, int row, int col, bool *visited)
{
    if (visited[row * sheet->totalCols + col])
        return true;
    visited[row * sheet->totalCols + col] = true;

    for (DependencyNode *curr = sheet->cells[row][col].dependencies; curr; curr = curr->next)
    {
        if (dfs_cycle_check(sheet, curr->row, curr->col, visited))
        {
            return true;
        }
    }
    return false;
}

bool detect_cycle(Spreadsheet *sheet, int row, int col)
{
    bool *visited = calloc(sheet->totalRows * sheet->totalCols, sizeof(bool));
    bool has_cycle = dfs_cycle_check(sheet, row, col, visited);
    free(visited);
    return has_cycle;
}

void clear_dependencies(Spreadsheet *sheet, int row, int col)
{
    // Clear dependencies for this cell
    DependencyNode *curr = sheet->cells[row][col].dependencies;
    while (curr)
    {
        DependencyNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    sheet->cells[row][col].dependencies = NULL;

    // Remove from others' dependents
    for (int i = 0; i < sheet->totalRows; i++)
    {
        for (int j = 0; j < sheet->totalCols; j++)
        {
            DependencyNode **pp = &sheet->cells[i][j].dependents;
            while (*pp)
            {
                if ((*pp)->row == row && (*pp)->col == col)
                {
                    DependencyNode *to_free = *pp;
                    *pp = (*pp)->next;
                    free(to_free);
                }
                else
                {
                    pp = &(*pp)->next;
                }
            }
        }
    }
}