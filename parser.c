#include "parser.h"
#include "spreadsheet.h"
#include "frontend.h"
#include "backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

static int col_label_to_index(const char *label)
{
    int result = 0;
    char c;
    for (int i = 0; label[i] != '\0'; i++)
    {
        if (!isalpha(label[i]) || islower(c = label[i]))
        {
            return -1;
        }
        result = result * 26 + (c - 'A' + 1);
    }
    return result - 1;
}

static int parse_cell_address(Spreadsheet *sheet, const char **input, int *row, int *col)
{
    char col_part[4] = {0};
    int i = 0;

    while (isalpha(**input) && i < 3)
    {
        col_part[i++] = **input;
        (*input)++;
    }
    if (i == 0)
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    } // No column letters

    // Convert column to index
    *col = col_label_to_index(col_part);
    if (*col <= 0 || *col > MAX_COLS)
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }

    *row = 0;
    while (isdigit(**input))
    {
        *row = *row * 10 + (**input - '0');
        (*input)++;
    }
    if (*row <= 0 || *row > MAX_ROWS)
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }
    *row -= 1;
    return 0;
}

// Returns true if the string represents a valid function name
static bool is_function(const char *str)
{
    static const char *functions[] = {"MIN", "MAX", "AVG", "SUM", "STDEV", "SLEEP"};
    for (int i = 0; i < 6; i++)
    {
        if (strncmp(str, functions[i], strlen(functions[i])) == 0)
        {
            return true;
        }
    }
    return false;
}

int is_valid_formula(const char *formula) {
    // Regex pattern that matches one of the fixed formula names,
    // followed by '(' with any characters inside and a closing ')'
    const char *pattern = "^(MIN|MAX|AVG|SUM|STDEV|SLEEP)\\(.*\\)$";
    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret) {
        // Handle error in regex compilation as needed
        return 0;
    }
    ret = regexec(&regex, formula, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

// Returns true if the character is an arithmetic operator
static bool is_operator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/';
}

// Convert operator character to Operation enum
static Operation char_to_operation(char c)
{
    switch (c)
    {
    case '+':
        return OP_ADD;
    case '-':
        return OP_SUB;
    case '*':
        return OP_MUL;
    case '/':
        return OP_DIV;
    default:
        return OP_NONE;
    }
}

// Parse a range (e.g., "A1:B2") and store cells in the range array
static int parse_range(Spreadsheet *sheet, const char *range_str, Cell ***range_cells, int *range_size)
{
    char *colon = strchr(range_str, ':');
    // Split the range into start and end
    int len = strlen(range_str);
    char *range_copy = malloc(len + 1);
    strcpy(range_copy, range_str);
    range_copy[colon - range_str] = '\0';

    // Parse start and end cells
    int start_row, start_col, end_row, end_col;
    const char *start_ptr = range_copy;
    const char *end_ptr = colon + 1;

    if (parse_cell_address(sheet, &start_ptr, &start_row, &start_col) != 0 ||
        parse_cell_address(sheet, &end_ptr, &end_row, &end_col) != 0)
    {
        free(range_copy);
        return -1;
    }

    // Validate range order
    if (start_row > end_row || start_col > end_col)
    {
        sheet->last_status = ERR_INVALID_RANGE;
        free(range_copy);
        return -1;
    }

    // Calculate range size and allocate memory
    int rows = end_row - start_row + 1;
    int cols = end_col - start_col + 1;
    *range_size = rows * cols;

// Maybe make changes here so as to add the cells into dependencies and not as a matrix
    *range_cells = malloc(sizeof(Cell *) * (*range_size));

    // Fill the range array
    int idx = 0;
    for (int r = start_row; r <= end_row; r++)
    {
        for (int c = start_col; c <= end_col; c++)
        {
            (*range_cells)[idx++] = &sheet->cells[r][c];
        }
    }

    free(range_copy);
    return 0;
}

// Parse arithmetic expression (e.g., "A1+2" or "B2*C3")
static int parse_arithmetic(Spreadsheet *sheet, Cell *target_cell, const char *formula)
{
    regex_t regex;
    // Pattern explanation:
    //   ^\s*                         : start of string, optional whitespace
    //   ([-+]?[0-9]+|[A-Z]+[0-9]+)    : operand1 (a constant with optional sign or a cell reference)
    //   \s*                          : optional whitespace
    //   ([+\-*/])                   : operator: one of +, -, *, or /
    //   \s*                          : optional whitespace
    //   ([-+]?[0-9]+|[A-Z]+[0-9]+)    : operand2 (again, a constant or a cell reference)
    //   \s*$                         : optional whitespace until end of string
    const char *pattern = "^\\s*([-+]?[0-9]+|[A-Z]+[0-9]+)\\s*([+\\-*/])\\s*([-+]?[0-9]+|[A-Z]+[0-9]+)\\s*$";
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0)
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }

    regmatch_t matches[4]; // matches[0]: full match, [1]: operand1, [2]: operator, [3]: operand2
    int ret = regexec(&regex, formula, 4, matches, 0);
    regfree(&regex);
    if (ret != 0)
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }

    // Extract operand1
    int len = matches[1].rm_eo - matches[1].rm_so;
    char operand1[64] = {0};
    strncpy(operand1, formula + matches[1].rm_so, len);
    operand1[len] = '\0';

    // Extract operator
    len = matches[2].rm_eo - matches[2].rm_so;
    char op_str[4] = {0};
    strncpy(op_str, formula + matches[2].rm_so, len);
    op_str[len] = '\0';

    // Extract operand2
    len = matches[3].rm_eo - matches[3].rm_so;
    char operand2[64] = {0};
    strncpy(operand2, formula + matches[3].rm_so, len);
    operand2[len] = '\0';

    target_cell->type = TYPE_ARITHMETIC;
    target_cell->op_data.arithmetic.op = char_to_operation(op_str[0]);
    target_cell->op_data.arithmetic.constant = 0;

    // Process operand1
    int type1, value1, row1, col1;
    type1 = check_constant_or_cell_address(operand1, &value1, &row1, &col1);
    if (type1 == 0)
    {
        // Operand is a numeric constant.
        target_cell->op_data.arithmetic.operand1 = NULL;
        target_cell->op_data.arithmetic.constant += value1;
    }
    else if (type1 == 1)
    {
        // Operand is a cell reference.
        target_cell->op_data.arithmetic.operand1 = &sheet->cells[row1][col1];

        // Update dependency: target_cell depends on the referenced cell.
        if (target_cell->dependencies == NULL)
            target_cell->dependencies = create_cellset();
        cellset_insert(target_cell->dependencies, target_cell->op_data.arithmetic.operand1);

        // Update the referenced cell’s dependents: add target_cell.
        if (target_cell->op_data.arithmetic.operand1->dependents == NULL)
            target_cell->op_data.arithmetic.operand1->dependents = create_cellset();
        cellset_insert(target_cell->op_data.arithmetic.operand1->dependents, target_cell);
    }
    else
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }

    // Process operand2
    int type2, value2, row2, col2;
    type2 = check_constant_or_cell_address(operand2, &value2, &row2, &col2);
    if (type2 == 0)
    {
        target_cell->op_data.arithmetic.operand2 = NULL;
        target_cell->op_data.arithmetic.constant += value2;
    }
    else if (type2 == 1)
    {
        target_cell->op_data.arithmetic.operand2 = &sheet->cells[row2][col2];

        // Update dependency: target_cell depends on this referenced cell.
        if (target_cell->dependencies == NULL)
            target_cell->dependencies = create_cellset();
        cellset_insert(target_cell->dependencies, target_cell->op_data.arithmetic.operand2);

        // And update the referenced cell's dependents.
        if (target_cell->op_data.arithmetic.operand2->dependents == NULL)
            target_cell->op_data.arithmetic.operand2->dependents = create_cellset();
        cellset_insert(target_cell->op_data.arithmetic.operand2->dependents, target_cell);
    }
    else
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }

    if (type1 == 0 && type2 == 0) {
        int evaluated;
        switch (target_cell->op_data.arithmetic.op) {
        case OP_ADD:
            evaluated = value1 + value2;
            break;
        case OP_SUB:
            evaluated = value1 - value2;
            break;
        case OP_MUL:
            evaluated = value1 * value2;
            break;
        case OP_DIV:
            if (value2 == 0) {
                sheet->last_status = ERR_DIV_ZERO;
                return -1;
            }
            evaluated = value1 / value2;
            break;
        default:
            sheet->last_status = ERR_SYNTAX;
            return -1;
        }
        target_cell->type = TYPE_CONSTANT;
        target_cell->value = evaluated;
    }
    return 0;
}

// Parse function call (e.g., "SUM(A1:B2)")
static int parse_function(Spreadsheet *sheet, Cell *target_cell, const char *formula)
{
    // Extract function name
    char func_name[10] = {0};
    const char *p = formula;
    int i = 0;
    while (*p && *p != '(' && i < 9)
    {
        func_name[i++] = *p++;
    }
    // Find closing parenthesis
    const char *close_paren = strchr(p, ')');
    if (!close_paren)
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }


    // Extract range/value
    int range_len = close_paren - p - 1;
    char *range_str = malloc(range_len + 1);
    strncpy(range_str, p + 1, range_len);
    range_str[range_len] = '\0';

    target_cell->type = TYPE_FUNCTION;
    target_cell->op_data.function.func_name = strdup(func_name);

    // Parse range or single value

// Maybe handle sleep differently

    if (strchr(range_str, ':'))
    {
        if (parse_range(sheet, range_str, &target_cell->op_data.function.range,
                        &target_cell->op_data.function.range_size) != 0)
        {
            free(range_str);
            return -1;
        }
    }
    else
    {
        // Single cell or value
        if (isalpha(range_str[0]))
        {
            int row, col;
            const char *ptr = range_str;
            if (parse_cell_address(sheet, &ptr, &row, &col) != 0)
            {
                free(range_str);
                return -1;
            }
            target_cell->op_data.function.range = malloc(sizeof(Cell *));
            target_cell->op_data.function.range[0] = &sheet->cells[row][col];
            target_cell->op_data.function.range_size = 1;
        }
        else
        {
            // It's a constant value (e.g., SLEEP(2))
            target_cell->type = TYPE_CONSTANT;
            target_cell->value = atoi(range_str);
            free(range_str);
            return 0;
        }
    }

    free(range_str);
    // --- Update dependency sets for each cell in the function range ---
    for (int i = 0; i < target_cell->op_data.function.range_size; i++)
    {
        Cell *refCell = target_cell->op_data.function.range[i];
        // Insert refCell into target_cell's dependencies.
        if (target_cell->dependencies == NULL)
            target_cell->dependencies = create_cellset();
        cellset_insert(target_cell->dependencies, refCell);

        // Also, add target_cell as a dependent of refCell.
        if (refCell->dependents == NULL)
            refCell->dependents = create_cellset();
        cellset_insert(refCell->dependents, target_cell);
    }
    return 0;
}

// Check for cyclic dependencies
static bool check_cycle(Spreadsheet *sheet, Cell *start_cell, Cell *current_cell)
{
    if (current_cell->in_stack)
    {
        return true; // Cycle detected
    }

    if (current_cell->visited)
    {
        return false; // Already checked this path
    }

    current_cell->visited = true;
    current_cell->in_stack = true;

    // Check dependencies based on cell type
    if (current_cell->type == TYPE_ARITHMETIC)
    {
        if (current_cell->op_data.arithmetic.operand1 &&
            check_cycle(sheet, start_cell, current_cell->op_data.arithmetic.operand1))
        {
            return true;
        }
        if (current_cell->op_data.arithmetic.operand2 &&
            check_cycle(sheet, start_cell, current_cell->op_data.arithmetic.operand2))
        {
            return true;
        }
    }
    else if (current_cell->type == TYPE_FUNCTION)
    {
        for (int i = 0; i < current_cell->op_data.function.range_size; i++)
        {
            if (check_cycle(sheet, start_cell, current_cell->op_data.function.range[i]))
            {
                return true;
            }
        }
    }

    current_cell->in_stack = false;
    return false;
}
int check_constant_or_cell_address(const char *str, int *constant_value, int *row, int *col)
{
    // First, try to parse a constant.
    char *endptr;
    long num = strtol(str, &endptr, 10);
    if (*endptr == '\0')
    {
        // Entire string was consumed => valid constant.
        *constant_value = (int)num;
        return 0;
    }

    // Otherwise, check if the string is a valid cell address.
    // Expected format: one or more letters followed by one or more digits.
    int i = 0;
    while (str[i] && isalpha((unsigned char)str[i]))
    {
        i++;
    }
    if (i == 0)
    {
        // No alphabetic part, invalid cell address.
        return -1;
    }

    int j = i;
    while (str[j] && isdigit((unsigned char)str[j]))
    {
        j++;
    }
    if (str[j] != '\0')
    {
        // Extra characters detected.
        return -1;
    }

    // Convert letter(s) to a column index (A => 0, B => 1, etc.).
    int col_num = 0;
    for (int k = 0; k < i; k++)
    {
        col_num = col_num * 26 + (toupper(str[k]) - 'A' + 1);
    }
    // Adjust for 0-indexing.
    *col = col_num - 1;

    // Convert the numeric part to a row index (1-based to 0-based).
    int row_num = atoi(str + i);
    if (row_num <= 0)
    {
        // Invalid row number.
        return -1;
    }
    *row = row_num - 1;

    return 1;
}

// Main formula parsing function
int evaluate_expression(const char *expr, int *result);
int parse_formula(Spreadsheet *sheet, Cell *cell, const char *formula)
{
    // Reset cell's current state
    cell->has_error = false;
    free(cell->error_msg);
    cell->error_msg = NULL;

    /*                                      Check if the value is a single constant                                        */
    bool is_numeric = true;
    int i = 0;
    // Allow a leading '-' for negative numbers
    if (formula[0] == '-')
        i = 1;
    for (; formula[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)formula[i]))
        {
            is_numeric = false;
            break;
        }
    }

    if (is_numeric)
    {
        int value = atoi(formula);
        cell->type = TYPE_CONSTANT;
        cell->value = value;
        return 0;
    }

    /*                                      Check if the input is a formula                                                      */
    if (is_valid_formula(formula))
    {
        if (parse_function(sheet, cell, formula) != 0)
        {
            return -1;
        }
    }
    /*                                      Check if it's an arithmetic expression                                                */
    else if (strchr(formula, '+') || strchr(formula, '-') ||
             strchr(formula, '*') || strchr(formula, '/'))
    {
        if (parse_arithmetic(sheet, cell, formula) != 0)
        {
            return -1;
        }
    }
    // Must be a cell reference
    else if (isalpha(formula[0]))
    {
        int row, col;
        const char *ptr = formula;
        if (parse_cell_address(sheet, &ptr, &row, &col) != 0)
        {
            return -1;
        }
        cell->type = TYPE_REFERENCE;
        cell->op_data.arithmetic.operand1 = &sheet->cells[row][col];

        // --- Update dependency: the current cell depends on the referenced cell ---
        if (cell->dependencies == NULL)
            cell->dependencies = create_cellset();
        cellset_insert(cell->dependencies, &sheet->cells[row][col]);
        if (sheet->cells[row][col].dependents == NULL)
            sheet->cells[row][col].dependents = create_cellset();
        cellset_insert(sheet->cells[row][col].dependents, cell);
    }
    else
    {
        sheet->last_status = ERR_SYNTAX;
        return -1;
    }

    // Check for cyclic dependencies
    // Reset visited flags first
    for (int r = 0; r < sheet->totalRows; r++)
    {
        for (int c = 0; c < sheet->totalCols; c++)
        {
            sheet->cells[r][c].visited = false;
            sheet->cells[r][c].in_stack = false;
        }
    }

    if (check_cycle(sheet, cell, cell))
    {
        sheet->last_status = ERR_CIRCULAR_REFERENCE;
        return -1;
    }

    return 0;
}
int evaluate_expression(const char *expr, int *result)
{
    // Implement a simple expression evaluator here
    // For now, let's assume it returns 0 on success and sets *result
    // You can use a library or write your own parser for this
    // Example: *result = some_evaluation_function(expr);
    return 0;
}

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

    *eq_pos = '\0'; 
    char *cellRef = input;
    char *formula = eq_pos + 1;

    int row, col;
    if (parse_cell_address(sheet, cellRef, &row, &col) != 0)
    {
        // raise error as the syntax maybe incorrect
        return;
    }

    // THis maybe not needed as we checked while parsing the cell
    if (row >= sheet->totalRows || col >= sheet->totalCols)
    {
        sheet->last_status = ERR_INVALID_CELL;
        return;
    }
    /*              Cell is successfully parsed, now we go to the RHS                      */

    end = formula + strlen(formula) - 1;
    while (end > formula && *end == ' ')
        *end-- = '/0';

    if (strlen(formula) == 0)
    {
        sheet->last_status = ERR_SYNTAX;
        return;
    }

    Cell *target_cell = &sheet->cells[row][col];

    // Here some update is needed to also store the dependents and dependencies

    // Backup the current state of the cell
    char *old_formula = target_cell->formula ? strdup(target_cell->formula) : NULL;
    int old_value = target_cell->value;
    CellType old_type = target_cell->type;

    // Tentatively set the new formula
    char *new_formula = strdup(formula);
    target_cell->formula = new_formula;

    // Attempt to parse and validate the new formula, which also checks for cycles
    if (parse_formula(sheet, target_cell, formula) != 0)
    {
        // An error occurred (syntax or circular dependency)
        free(target_cell->formula);
        target_cell->formula = old_formula;
        target_cell->value = old_value;
        target_cell->type = old_type;
        // will update the previous dependents and dependencies
        return;
    }
    else
    {
        free(old_formula);
    }

// Check here

    // Mark cell and its dependents for recalculation
    mark_for_recalculation(sheet, target_cell);

    // Perform the recalculation
    recalculate_sheet(sheet);

    sheet->last_status = STATUS_OK;
}