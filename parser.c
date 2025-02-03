#include "parser.h"
#include "spreadsheet.h"
#include "frontend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Helper: Convert a column label (e.g., "A", "AA") into a zero-indexed column number.
 */
static int col_label_to_index(const char *colLabel)
{
    int col = 0;
    for (int i = 0; colLabel[i] != '\0'; i++)
    {
        if (!isupper(*colLabel))
        {
            return -1;
        }
        col = col * 26 + (colLabel[i] - 'A' + 1);
    }
    return col - 1;
}

/*
 * Helper: Parse a cell address (e.g., "B2") into zero-indexed row and column values.
 * Returns 0 on success, or -1 on failure.
 */
static int parse_cell_address(const char *str, int *row, int *col)
{
    if (!str || *str == '\0')
        return -1; // Empty input

    int i = 0, j = 0;
    char colLabel[10] = {0};

    // Parse alphabetic column label
    while (isalpha((unsigned char)str[i]) && j < (int)sizeof(colLabel) - 1)
    {
        // if (j >= (int)sizeof(colLabel) - 1)
        //     return -1;
        colLabel[j++] = toupper((unsigned char)str[i]);
        i++;
    }
    colLabel[j] = '\0';

    if (j == 0)
        return -1; // No column letters found

    // Parse numeric row portion
    if (!isdigit((unsigned char)str[i]))
        return -1;
    int num = 0;
    while (isdigit((unsigned char)str[i]))
    {
        num = num * 10 + (str[i] - '0');
        i++;
    }
    if (str[i] != '\0' || num < 1 || num > MAX_ROWS)
        return -1; // Extra characters in the cell address

    *col = col_label_to_index(colLabel);
    *row = num - 1;
    return (*col >= 0 && *col < MAX_COLS) ? 0 : -1;
}

/*
 * process_command:
 *   This function dispatches the user command. It first checks if the input
 *   is a scroll command. Otherwise, it assumes the input is a formula command.
 */
void process_command(Spreadsheet *sheet, char *input)
{
    if (!input || *input == '\0')
    {
        sheet->last_status = ERR_SYNTAX;
        return;
    }

    // Trim leading/trailing spaces
    while (*input == ' ')
        input++;
    char *end = input + strlen(input) - 1;
    while (end > input && *end == ' ')
        *end-- = '\0';

    char *eq_pos = strchr(input, '=');
    if (!eq_pos)
    {
        sheet->last_status = ERR_SYNTAX;
        return;
    }

    // Split into cell reference (left) and formula/value (right)
    *eq_pos = '\0'; // Terminate cell reference string
    char *cellRef = input;
    char *formula = eq_pos + 1;

    // Trim spaces again
    while (*cellRef == ' ')
        cellRef++;
    while (*formula == ' ')
        formula++;

    int row, col;
    if (parse_cell_address(cellRef, &row, &col) != 0 || row >= sheet->totalRows || col >= sheet->totalCols)
    {
        sheet->last_status = ERR_INVALID_CELL;
        return;
    }

    // Clear existing dependencies
    clear_dependencies(sheet, row, col);

    // If the formula is a direct number (e.g., "A1=5"), store as value
    char *endptr;
    int num_value = strtol(formula, &endptr, 10);
    if (*endptr == '\0') // If entire string is a number
    {
        sheet->cells[row][col].isFormula = false;
        sheet->cells[row][col].content = num_value;
        sheet->last_status = STATUS_OK;
        return;
    }

    // Otherwise, store as a formula
    strncpy(sheet->cells[row][col].formula, formula, MAX_FORMULA_LEN - 1);
    sheet->cells[row][col].formula[MAX_FORMULA_LEN - 1] = '\0';
    sheet->cells[row][col].isFormula = true;
    sheet->cells[row][col].needs_recalc = true;

    /*
     * TODO: Parse the formula further to extract cell references and add dependencies
     * using add_dependency() for each referenced cell.
     */

    // Evaluate the formula
    CalcStatus status;
    int new_val = evaluate_expression(sheet, row, col, sheet->cells[row][col].formula, &status);
    if (status == STATUS_OK)
    {
        sheet->cells[row][col].content = new_val;
    }
    sheet->last_status = status;
}
