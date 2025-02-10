#include "spreadsheet.h"
#include "frontend.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>

Spreadsheet *create_spreadsheet(int rows, int cols)
{
    Spreadsheet *sheet = malloc(sizeof(Spreadsheet));
    sheet->totalRows = rows;
    sheet->totalCols = cols;
    sheet->scroll_row = 0;
    sheet->scroll_col = 0;
    sheet->mode = VIEW_MODE;
    sheet->last_status = STATUS_OK;

    sheet->cells = malloc(rows * sizeof(Cell *));
    for (int i = 0; i < rows; i++)
    {
        sheet->cells[i] = calloc(cols, sizeof(Cell));
        for (int j = 0; j < cols; j++)
        {
            sheet->cells[i][j] = (Cell){
                .value = 0,
                .formula = NULL,
                .type = TYPE_EMPTY,
                .dependents = NULL,
                .dep_count = 0,
                .dependencies = NULL,
                .depd_count = 0,
                .has_error = false,
                .error_msg = NULL,
                .visited = false,
                .in_stack = false,
                .topo_order = -1};
        }
    }
    return sheet;
}

void editCell(Spreadsheet *sheet)
{
    printf("Enter command: ");
    restore_terminal();

    char input_line[MAX_CELL_LENGTH];
    fgets(input_line, MAX_CELL_LENGTH, stdin);
    input_line[strcspn(input_line, "\n")] = 0;

    if (strlen(input_line) > 0)
    {
        process_command(sheet, input_line);
    }
    // recalculate(sheet);
    configure_terminal();
}

void destroy_spreadsheet(Spreadsheet *sheet)
{
    for (int i = 0; i < sheet->totalRows; i++)
    {
        for (int j = 0; j < sheet->totalCols; j++)
        {
            // clear_dependencies(sheet, i, j);
        }
        free(sheet->cells[i]);
    }
    free(sheet->cells);
    free(sheet);
}

void clear_dependencies(Spreadsheet *sheet, int row, int col)
{
    CellRef target = {row, col};

    // Clear from dependents' dependency lists
    CellRefIterator *it = cellref_iterator(sheet->cells[row][col].dependents);
    while (cellref_iterator_has_next(it))
    {
        CellRef dep = cellref_iterator_next(it);
        cellref_set_remove(sheet->cells[dep.row][dep.col].dependencies, target);
    }
    cellref_iterator_free(it);

    // Clear local sets
    cellref_set_destroy(sheet->cells[row][col].dependents);
    cellref_set_destroy(sheet->cells[row][col].dependencies);
    sheet->cells[row][col].dependents = cellref_set_create(cellref_compare);
    sheet->cells[row][col].dependencies = cellref_set_create(cellref_compare);
}
