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
                .dependencies = NULL,
                .has_error = false,
                .error_msg=NULL,
                .visited = false,
                .in_stack=false,
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
    recalculate(sheet);
    configure_terminal();
}

void destroy_spreadsheet(Spreadsheet *sheet)
{
    for (int i = 0; i < sheet->totalRows; i++)
    {
        for (int j = 0; j < sheet->totalCols; j++)
        {
            clear_dependencies(sheet, i, j);
        }
        free(sheet->cells[i]);
    }
    free(sheet->cells);
    free(sheet);
}
