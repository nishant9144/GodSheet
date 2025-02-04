#include "parser.h"
#include "spreadsheet.h"
#include "frontend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static ExprNode *parse_expression(const char **input);
static ExprNode *parse_term(const char **input);
static ExprNode *parse_factor(const char **input);
static ExprNode *parse_primary(const char **input);

static void skip_whitespace(const char **input) {
    while (isspace(**input)) (*input)++;
}

static int parse_cell_reference(const char **input, int *row, int *col) {
    char col_part[4] = {0};
    int i = 0;
    
    // Parse column letters
    while (isalpha(**input) && i < 3) {
        col_part[i++] = **input;
        (*input)++;
    }
    if (i == 0) return 0;  // No column letters
    
    // Convert column to index
    *col = col_label_to_index(col_part);
    if (*col < 0) return 0;
    
    // Parse row numbers
    *row = 0;
    while (isdigit(**input)) {
        *row = *row * 10 + (**input - '0');
        (*input)++;
    }
    if (*row == 0) return 0;  // No row numbers
    
    *row -= 1;  // Convert to 0-based index
    return 1;
}

static ExprNode *create_constant(int value) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_CONSTANT;
    node->constant = value;
    return node;
}

static ExprNode *create_reference(int row, int col) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_REFERENCE;
    node->reference.row = row;
    node->reference.col = col;
    return node;
}

static ExprNode *create_operator(char op, ExprNode *left, ExprNode *right) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_OPERATOR;
    node->operation.op = op;
    node->operation.left = left;
    node->operation.right = right;
    return node;
}

static ExprNode *create_function(const char *name, ExprNode *range) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_FUNCTION;
    node->function.name = strdup(name);
    node->function.args = malloc(sizeof(ExprNode *));
    node->function.args[0] = range;
    node->function.args_count = 1;
    return node;
}

static ExprNode *create_range(ExprNode *start, ExprNode *end) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_RANGE;
    node->range.start = start;
    node->range.end = end;
    return node;
}

static int validate_range(ExprNode *start, ExprNode *end) {
    if (start->type != NODE_REFERENCE || end->type != NODE_REFERENCE) return 0;
    return (start->reference.row <= end->reference.row) &&
           (start->reference.col <= end->reference.col);
}

ExprNode *parse_expression(const char **input) {
    ExprNode *left = parse_term(input);
    while (1) {
        skip_whitespace(input);
        if (**input == '+' || **input == '-') {
            char op = **input;
            (*input)++;
            ExprNode *right = parse_term(input);
            left = create_operator(op, left, right);
        } else {
            break;
        }
    }
    return left;
}

ExprNode *parse_term(const char **input) {
    ExprNode *left = parse_factor(input);
    while (1) {
        skip_whitespace(input);
        if (**input == '*' || **input == '/') {
            char op = **input;
            (*input)++;
            ExprNode *right = parse_factor(input);
            left = create_operator(op, left, right);
        } else {
            break;
        }
    }
    return left;
}

ExprNode *parse_factor(const char **input) {
    skip_whitespace(input);
    if (**input == '(') {
        (*input)++;
        ExprNode *expr = parse_expression(input);
        skip_whitespace(input);
        if (**input != ')') {
            free_expr_tree(expr);
            return NULL;
        }
        (*input)++;
        return expr;
    }
    return parse_primary(input);
}

ExprNode *parse_primary(const char **input) {
    skip_whitespace(input);
    
    // Number constant
    if (isdigit(**input)) {
        int value = 0;
        while (isdigit(**input)) {
            value = value * 10 + (**input - '0');
            (*input)++;
        }
        return create_constant(value);
    }
    
    // Cell reference or function
    if (isalpha(**input)) {
        const char *start = *input;
        while (isalnum(**input)) (*input)++;
        size_t length = *input - start;
        
        // Check if it's a function
        if (**input == '(') {
            char func_name[10] = {0};
            strncpy(func_name, start, length);
            (*input)++;  // Skip '('
            
            ExprNode *arg = parse_primary(input);
            if (arg == NULL) return NULL;
            
            // Validate it's a range
            if (arg->type != NODE_RANGE || !validate_range(arg->range.start, arg->range.end)) {
                free_expr_tree(arg);
                return NULL;
            }
            
            skip_whitespace(input);
            if (**input != ')') {
                free_expr_tree(arg);
                return NULL;
            }
            (*input)++;  // Skip ')'
            
            return create_function(func_name, arg);
        }
        
        // Parse as cell reference or range
        *input = start;  // Reset to start of reference
        int row1, col1, row2, col2;
        if (!parse_cell_reference(input, &row1, &col1)) return NULL;
        
        // Check for range
        if (**input == ':') {
            (*input)++;  // Skip ':'
            if (!parse_cell_reference(input, &row2, &col2)) return NULL;
            ExprNode *start = create_reference(row1, col1);
            ExprNode *end = create_reference(row2, col2);
            if (!validate_range(start, end)) {
                free_expr_tree(start);
                free_expr_tree(end);
                return NULL;
            }
            return create_range(start, end);
        }
        
        return create_reference(row1, col1);
    }
    
    return NULL;  // Invalid syntax
}

ExprNode *parse_formula(const char *formula, CalcStatus *status) {
    const char *input = formula;
    ExprNode *result = parse_expression(&input);
    
    if (result == NULL) {
        *status = ERR_SYNTAX;
        return NULL;
    }
    
    skip_whitespace(&input);
    if (*input != '\0') {
        free_expr_tree(result);
        *status = ERR_SYNTAX;
        return NULL;
    }
    
    *status = STATUS_OK;
    return result;
}

void free_expr_tree(ExprNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_OPERATOR:
            free_expr_tree(node->operation.left);
            free_expr_tree(node->operation.right);
            break;
        case NODE_FUNCTION:
            free(node->function.name);
            free_expr_tree(node->function.args[0]);
            free(node->function.args);
            break;
        case NODE_RANGE:
            free_expr_tree(node->range.start);
            free_expr_tree(node->range.end);
            break;
        default: break;
    }
    
    free(node);
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

    // Split into cell reference (left) and formula/value (right)
    *eq_pos = '\0'; // Terminate cell reference string
    char *cellRef = input;
    char *formula = eq_pos + 1;

    int row, col;
    if (parse_cell_address(sheet, cellRef, &row, &col) != 0)
    {
        return;
    }
    if (row >= sheet->totalRows || col >= sheet->totalCols)
    {
        sheet->last_status = ERR_INVALID_CELL;
        return;
    }

    // Clear existing dependencies
    clear_dependencies(sheet, row, col);

    CalcStatus parse_status;
    ExprNode *expr = parse_formula(formula, &parse_status);
    
    if (parse_status != STATUS_OK) {
        sheet->last_status = parse_status;
        free_expr_tree(expr);
        return;
    }

    // Store parsed expression tree
    free_expr_tree(sheet->cells[row][col].expr_tree);
    sheet->cells[row][col].expr_tree = expr;

    // Collect dependencies
    clear_dependencies(sheet, row, col);
    collect_dependencies(expr, sheet, row, col);

    // Check for cycles
    if (detect_cycle(sheet, row, col)) {
        sheet->last_status = ERR_CIRCULAR_REF;
        return;
    }

    // Evaluate and update
    CalcStatus eval_status;
    int value = evaluate_expression(sheet, row, col, &eval_status);
    
    if (eval_status == STATUS_OK) {
        sheet->cells[row][col].content = value;
        mark_dependents_for_recalc(sheet, row, col);
    }
    
    sheet->last_status = eval_status;
}

ExprNode *create_constant_node(int value) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_CONSTANT;
    node->constant = value;
    return node;
}

// Create a cell reference node
ExprNode *create_reference_node(int row, int col) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_REFERENCE;
    node->reference.row = row;
    node->reference.col = col;
    return node;
}

// Create an operator node
ExprNode *create_operator_node(char op, ExprNode *left, ExprNode *right) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_OPERATOR;
    node->operation.op = op;
    node->operation.left = left;
    node->operation.right = right;
    return node;
}

// Create a function node
ExprNode *create_function_node(const char *name, ExprNode *range_arg) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_FUNCTION;
    node->function.name = strdup(name);
    node->function.args = malloc(sizeof(ExprNode *));
    node->function.args[0] = range_arg;
    node->function.args_count = 1;
    return node;
}

// Create a range node
ExprNode *create_range_node(ExprNode *start, ExprNode *end) {
    ExprNode *node = malloc(sizeof(ExprNode));
    node->type = NODE_RANGE;
    node->range.start = start;
    node->range.end = end;
    return node;
}

// Free the entire expression tree
void free_expr_tree(ExprNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_OPERATOR:
            free_expr_tree(node->operation.left);
            free_expr_tree(node->operation.right);
            break;
            
        case NODE_FUNCTION:
            free(node->function.name);
            for (int i = 0; i < node->function.args_count; i++) {
                free_expr_tree(node->function.args[i]);
            }
            free(node->function.args);
            break;
            
        case NODE_RANGE:
            free_expr_tree(node->range.start);
            free_expr_tree(node->range.end);
            break;
            
        default:
            break;
    }
    
    free(node);
}