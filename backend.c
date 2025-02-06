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
                .formula = NULL,
                .expr_tree = NULL,
                .dependents = NULL,
                .dependencies = NULL,
                .needs_recalc = false,
                .has_error = false};
        }
    }
    return sheet;
}

static void add_dependency(Spreadsheet *sheet, int src_row, int src_col, int dest_row, int dest_col);
static void collect_dependencies(ExprNode *node, Spreadsheet *sheet, int dest_row, int dest_col);
static int eval_node(Spreadsheet *sheet, ExprNode *node, int curr_row, int curr_col, CalcStatus *status);

void add_dependency(Spreadsheet *sheet, int src_row, int src_col, int dest_row, int dest_col)
{
    // Add to source cell's dependents
    DependencyNode *dep = malloc(sizeof(DependencyNode));
    dep->row = dest_row;
    dep->col = dest_col;
    dep->next = sheet->cells[src_row][src_col].dependents;
    sheet->cells[src_row][src_col].dependents = dep;

    // Add to destination cell's dependencies
    dep = malloc(sizeof(DependencyNode));
    dep->row = src_row;
    dep->col = src_col;
    dep->next = sheet->cells[dest_row][dest_col].dependencies;
    sheet->cells[dest_row][dest_col].dependencies = dep;
}

static void collect_dependencies(ExprNode *node, Spreadsheet *sheet, int dest_row, int dest_col)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_REFERENCE:
        add_dependency(sheet, node->reference.row, node->reference.col, dest_row, dest_col);
        break;

    case NODE_OPERATOR:
        collect_dependencies(node->operation.left, sheet, dest_row, dest_col);
        collect_dependencies(node->operation.right, sheet, dest_row, dest_col);
        break;

    case NODE_FUNCTION:
        for (int i = 0; i < node->function.args_count; i++)
        {
            collect_dependencies(node->function.args[i], sheet, dest_row, dest_col);
        }
        break;

    default:
        break;
    }
}

// int evaluate_expression(Spreadsheet *sheet, int row, int col, const char *formula, CalcStatus *status)
// {
//     Cell *cell = &sheet->cells[row][col];
//     cell->has_error = false;

//     // Parse the formula
//     const char *p = formula;
//     ExprNode *expr = parse_expression(&p);
//     if (!expr || *p != '\0')
//     {
//         *status = ERR_SYNTAX;
//         return 0;
//     }

//     // Clear old dependencies and collect new ones
//     clear_dependencies(sheet, row, col);
//     collect_dependencies(expr, sheet, row, col);

//     // Check for circular dependencies
//     if (detect_cycle(sheet, row, col))
//     {
//         *status = ERR_CIRCULAR_REF;
//         free_expr_tree(expr);
//         return 0;
//     }

//     // Evaluate
//     int result = eval_node(sheet, expr, row, col, status);
//     free_expr_tree(expr);

//     if (*status != STATUS_OK)
//     {
//         cell->has_error = true;
//         snprintf(cell->error_msg, sizeof(cell->error_msg), "ERR: %d", *status);
//     }
//     return result;
// }

// static int eval_node(Spreadsheet *sheet, ExprNode *node, int curr_row, int curr_col, CalcStatus *status)
// {
//     if (!node || *status != STATUS_OK)
//         return 0;

//     switch (node->type)
//     {
//     case NODE_CONSTANT:
//         return node->constant;

//     case NODE_REFERENCE:
//     {
//         Cell *ref_cell = &sheet->cells[node->reference.row][node->reference.col];
//         if (ref_cell->has_error)
//         {
//             *status = ERR_INVALID_CELL;
//             return 0;
//         }
//         return ref_cell->content;
//     }

//     case NODE_OPERATOR:
//     {
//         int left = eval_node(sheet, node->operation.left, curr_row, curr_col, status);
//         int right = eval_node(sheet, node->operation.right, curr_row, curr_col, status);
//         if (*status != STATUS_OK)
//             return 0;

//         switch (node->operation.op)
//         {
//         case '+':
//             return left + right;
//         case '-':
//             return left - right;
//         case '*':
//             return left * right;
//         case '/':
//             if (right == 0)
//             {
//                 *status = ERR_DIV_ZERO;
//                 return 0;
//             }
//             return left / right;
//         default:
//             *status = ERR_SYNTAX;
//             return 0;
//         }
//     }

//     case NODE_FUNCTION:
//     {
//         if (strcasecmp(node->function.name, "SUM") == 0)
//         {
//             int sum = 0;
//             for (int i = 0; i < node->function.args_count; i++)
//             {
//                 sum += eval_node(sheet, node->function.args[i], curr_row, curr_col, status);
//             }
//             return sum;
//         }
//         // Add other functions here
//         *status = ERR_SYNTAX;
//         return 0;
//     }

//     default:
//         *status = ERR_SYNTAX;
//         return 0;
//     }
// }

void clear_dependencies(Spreadsheet *sheet, int row, int col)
{
    // Clear dependents from other cells
    DependencyNode *curr = sheet->cells[row][col].dependencies;
    while (curr)
    {
        DependencyNode *temp = curr;
        curr = curr->next;

        // Remove from source cell's dependents
        DependencyNode **pp = &sheet->cells[temp->row][temp->col].dependents;
        while (*pp)
        {
            if ((*pp)->row == row && (*pp)->col == col)
            {
                DependencyNode *to_free = *pp;
                *pp = (*pp)->next;
                free(to_free);
                break;
            }
            pp = &(*pp)->next;
        }
        free(temp);
    }
    sheet->cells[row][col].dependencies = NULL;
}

bool detect_cycle(Spreadsheet *sheet, int start_row, int start_col)
{
    bool *visited = calloc(sheet->totalRows * sheet->totalCols, sizeof(bool));
    int *stack = malloc(sheet->totalRows * sheet->totalCols * sizeof(int));
    int stack_ptr = 0;

    stack[stack_ptr++] = start_row * sheet->totalCols + start_col;

    while (stack_ptr > 0)
    {
        int current = stack[--stack_ptr];
        if (visited[current])
            return true;
        visited[current] = true;

        int row = current / sheet->totalCols;
        int col = current % sheet->totalCols;

        for (DependencyNode *dep = sheet->cells[row][col].dependencies; dep; dep = dep->next)
        {
            int idx = dep->row * sheet->totalCols + dep->col;
            if (idx == start_row * sheet->totalCols + start_col)
                return true;
            stack[stack_ptr++] = idx;
        }
    }

    free(visited);
    free(stack);
    return false;
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
// static void mark_for_recalc(Spreadsheet *sheet, int row, int col) __attribute__((unused));
// static void mark_for_recalc(Spreadsheet *sheet, int row, int col)
// {
//     if (sheet->cells[row][col].needs_recalc)
//         return;
//     sheet->cells[row][col].needs_recalc = true;

//     for (DependencyNode *curr = sheet->cells[row][col].dependents; curr; curr = curr->next)
//     {
//         mark_for_recalc(sheet, curr->row, curr->col);
//     }
// }

// void recalculate(Spreadsheet *sheet)static void mark_for_recalc(Spreadsheet *sheet, int row, int col) __attribute__((unused));
// static void mark_for_recalc(Spreadsheet *sheet, int row, int col)
// {
//     if (sheet->cells[row][col].needs_recalc)
//         return;
//     sheet->cells[row][col].needs_recalc = true;

//     for (DependencyNode *curr = sheet->cells[row][col].dependents; curr; curr = curr->next)
//     {
//         mark_for_recalc(sheet, curr->row, curr->col);
//     }
// }
// {
//     for (int i = 0; i < sheet->totalRows; i++)
//     {
//         for (int j = 0; j < sheet->totalCols; j++)
//         {
//             if (sheet->cells[i][j].needs_recalc)
//             {
//                 if (sheet->cells[i][j].formula[0] != '\0')
//                 {
//                     CalcStatus status;
//                     int new_val = evaluate_expression(sheet, i, j, sheet->cells[i][j].formula, &status);
//                     if (status == STATUS_OK)
//                     {
//                         sheet->cells[i][j].content = new_val;
//                     }
//                     sheet->last_status = status;
//                 }
//                 sheet->cells[i][j].needs_recalc = false;
//             }
//         }
//     }
// }
/* ==================== */
/* Evaluation Functions */
/* ==================== */

static int eval_node(Spreadsheet *sheet, ExprNode *node, int curr_row, int curr_col, CalcStatus *status);

int evaluate_expression(Spreadsheet *sheet, int row, int col, CalcStatus *status) {
    Cell *cell = &sheet->cells[row][col];
    *status = STATUS_OK;
    
    if (!cell->expr_tree) {
        return cell->content;  // Direct value cell
    }

    return eval_node(sheet, cell->expr_tree, row, col, status);
}

static int eval_reference(Spreadsheet *sheet, ExprNode *node, int curr_row, int curr_col, CalcStatus *status) {
    int ref_row = node->reference.row;
    int ref_col = node->reference.col;
    
    // Validate cell coordinates
    if (ref_row < 0 || ref_row >= sheet->totalRows || 
        ref_col < 0 || ref_col >= sheet->totalCols) {
        *status = ERR_INVALID_CELL;
        return 0;
    }

    Cell *ref_cell = &sheet->cells[ref_row][ref_col];
    
    // Recalculate if needed
    if (ref_cell->needs_recalc) {
        CalcStatus ref_status;
        int new_val = evaluate_expression(sheet, ref_row, ref_col, &ref_status);
        if (ref_status != STATUS_OK) {
            *status = ref_status;
            return 0;
        }
        ref_cell->content = new_val;
        ref_cell->needs_recalc = false;
    }
    
    return ref_cell->content;
}

static int eval_range_sum(Spreadsheet *sheet, ExprNode *start, ExprNode *end, CalcStatus *status) {
    int sum = 0;
    int start_row = start->reference.row;
    int start_col = start->reference.col;
    int end_row = end->reference.row;
    int end_col = end->reference.col;

    for (int r = start_row; r <= end_row; r++) {
        for (int c = start_col; c <= end_col; c++) {
            if (r >= sheet->totalRows || c >= sheet->totalCols) {
                *status = ERR_INVALID_RANGE;
                return 0;
            }
            
            Cell *cell = &sheet->cells[r][c];
            if (cell->needs_recalc) {
                CalcStatus cell_status;
                int val = evaluate_expression(sheet, r, c, &cell_status);
                if (cell_status != STATUS_OK) {
                    *status = cell_status;
                    return 0;
                }
                cell->content = val;
                cell->needs_recalc = false;
            }
            sum += cell->content;
        }
    }
    return sum;
}

static int eval_function(Spreadsheet *sheet, ExprNode *node, int curr_row, int curr_col, CalcStatus *status) {
    if (node->function.args_count != 1) {
        *status = ERR_SYNTAX;
        return 0;
    }

    ExprNode *arg = node->function.args[0];
    if (arg->type != NODE_RANGE) {
        *status = ERR_INVALID_RANGE;
        return 0;
    }

    // Validate range boundaries
    ExprNode *start = arg->range.start;
    ExprNode *end = arg->range.end;
    if (start->type != NODE_REFERENCE || end->type != NODE_REFERENCE) {
        *status = ERR_INVALID_RANGE;
        return 0;
    }

    if (start->reference.row > end->reference.row ||
        start->reference.col > end->reference.col) {
        *status = ERR_INVALID_RANGE;
        return 0;
    }

    if (strcasecmp(node->function.name, "SUM") == 0) {
        return eval_range_sum(sheet, start, end, status);
    }
    else if (strcasecmp(node->function.name, "AVG") == 0) {
        int sum = eval_range_sum(sheet, start, end, status);
        int count = (end->reference.row - start->reference.row + 1) *
                    (end->reference.col - start->reference.col + 1);
        return count > 0 ? sum / count : 0;
    }
    else if (strcasecmp(node->function.name, "MIN") == 0) {
        // Implementation similar to SUM but track minimum
    }
    // Implement other functions similarly

    *status = ERR_SYNTAX;
    return 0;
}

static int eval_node(Spreadsheet *sheet, ExprNode *node, int curr_row, int curr_col, CalcStatus *status) {
    if (!node || *status != STATUS_OK) return 0;

    switch (node->type) {
        case NODE_CONSTANT:
            return node->constant;

        case NODE_REFERENCE:
            return eval_reference(sheet, node, curr_row, curr_col, status);

        case NODE_OPERATOR: {
            int left = eval_node(sheet, node->operation.left, curr_row, curr_col, status);
            int right = eval_node(sheet, node->operation.right, curr_row, curr_col, status);
            
            if (*status != STATUS_OK) return 0;

            switch (node->operation.op) {
                case '+': return left + right;
                case '-': return left - right;
                case '*': return left * right;
                case '/':
                    if (right == 0) {
                        *status = ERR_DIV_ZERO;
                        return 0;
                    }
                    return left / right;
                default:
                    *status = ERR_SYNTAX;
                    return 0;
            }
        }

        case NODE_FUNCTION:
            return eval_function(sheet, node, curr_row, curr_col, status);

        default:
            *status = ERR_SYNTAX;
            return 0;
    }
}

/* ======================== */
/* Recalculation Functions */
/* ======================== */

void mark_dependents_for_recalc(Spreadsheet *sheet, int row, int col) {
    for (DependencyNode *dep = sheet->cells[row][col].dependents; dep != NULL; dep = dep->next) {
        if (!sheet->cells[dep->row][dep->col].needs_recalc) {
            sheet->cells[dep->row][dep->col].needs_recalc = true;
        }
    }
}

void recalculate(Spreadsheet *sheet) {
    bool changes_detected;
    do {
        changes_detected = false;
        for (int i = 0; i < sheet->totalRows; i++) {
            for (int j = 0; j < sheet->totalCols; j++) {
                Cell *cell = &sheet->cells[i][j];
                
                if (cell->needs_recalc) {
                    int old_value = cell->content;
                    CalcStatus status;
                    int new_val = evaluate_expression(sheet, i, j, &status);
                    
                    if (status == STATUS_OK) {
                        if (new_val != old_value) {
                            cell->content = new_val;
                            mark_dependents_for_recalc(sheet, i, j);
                            changes_detected = true;
                        }
                    } else {
                        cell->has_error = true;
                        snprintf(cell->error_msg, sizeof(cell->error_msg), "ERR: %d", status);
                    }
                    cell->needs_recalc = false;
                }
            }
        }
    } while (changes_detected);  // Repeat until no more changes
}