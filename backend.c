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
                .content = 0,
                .isFormula = false,
                .formula = {'\0'},
                .dependents = NULL,
                .dependencies = NULL,
                .needs_recalc = false};
        }
    }
    return sheet;
}

void editCell(Spreadsheet *sheet)
{
    printf("Enter command (e.g., A5=3 or A5=B1+B2): ");
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
static void mark_for_recalc(Spreadsheet *sheet, int row, int col) __attribute__((unused));
static void mark_for_recalc(Spreadsheet *sheet, int row, int col)
{
    if (sheet->cells[row][col].needs_recalc)
        return;
    sheet->cells[row][col].needs_recalc = true;

    for (DependencyNode *curr = sheet->cells[row][col].dependents; curr; curr = curr->next)
    {
        mark_for_recalc(sheet, curr->row, curr->col);
    }
}

void recalculate(Spreadsheet *sheet)
{
    for (int i = 0; i < sheet->totalRows; i++)
    {
        for (int j = 0; j < sheet->totalCols; j++)
        {
            if (sheet->cells[i][j].needs_recalc)
            {
                if (sheet->cells[i][j].formula[0] != '\0')
                {
                    CalcStatus status;
                    int new_val = evaluate_expression(sheet, i, j,
                                                      sheet->cells[i][j].formula,
                                                      &status);
                    if (status == STATUS_OK)
                    {
                        sheet->cells[i][j].content = new_val;
                    }
                    sheet->last_status = status;
                }
                sheet->cells[i][j].needs_recalc = false;
            }
        }
    }
}

int evaluate_expression(Spreadsheet *sheet, int row, int col, const char *formula, CalcStatus *status)
{
    (void)sheet;
    (void)row;
    (void)col;
    (void)formula;
    *status = STATUS_OK;
    return 0;
}
