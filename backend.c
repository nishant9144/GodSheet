#include "backend.h"
#include "datastructures.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Helper: Convert column string (e.g., "AA") to 0-based index
int col_to_index(const char *col_str) {
    int index = 0;
    while (*col_str) {
        index = index * 26 + (toupper(*col_str) - 'A' + 1);
        col_str++;
    }
    return index - 1;
}

// Helper: Parse cell reference like "A1"
bool parse_cell_ref(const char *str, CellRef *ref) {
    char col_str[4] = {0};
    int i = 0;
    
    // Extract column
    while (isalpha(str[i]) && i < 3) {
        col_str[i] = str[i];
        i++;
    }
    if (i == 0) return false;
    
    // Extract row
    int row = atoi(str + i) - 1;
    if (row < 0) return false;
    
    ref->col = col_to_index(col_str);
    ref->row = row;
    return true;
}

// Create/destroy spreadsheet
Spreadsheet* sheet_create(int rows, int cols) {
    Spreadsheet *sheet = malloc(sizeof(Spreadsheet));
    sheet->rows = rows;
    sheet->cols = cols;
    
    // Allocate cells
    sheet->cells = malloc(rows * sizeof(Cell*));
    for (int i = 0; i < rows; i++) {
        sheet->cells[i] = calloc(cols, sizeof(Cell));
    }
    
    // Initialize dependency graph
    sheet->dependency_graph = calloc(rows * cols, sizeof(bool*));
    for (int i = 0; i < rows * cols; i++) {
        sheet->dependency_graph[i] = calloc(rows * cols, sizeof(bool));
    }
    
    return sheet;
}

void sheet_destroy(Spreadsheet *sheet) {
    // Free cells
    for (int i = 0; i < sheet->rows; i++) {
        for (int j = 0; j < sheet->cols; j++) {
            free(sheet->cells[i][j].dependencies);
        }
        free(sheet->cells[i]);
    }
    free(sheet->cells);
    
    // Free dependency graph
    for (int i = 0; i < sheet->rows * sheet->cols; i++) {
        free(sheet->dependency_graph[i]);
    }
    free(sheet->dependency_graph);
    
    free(sheet);
}

// Set cell value (triggers recalc)
CalcError sheet_set_formula(Spreadsheet *sheet, int row, int col, const char *formula_str) {
    if (row < 0 || row >= sheet->rows || col < 0 || col >= sheet->cols) {
        return ERR_INVALID_REF;
    }

    Cell *cell = &sheet->cells[row][col];
    
    // Free existing formula if any
    if (cell->formula) {
        free_ast(cell->formula);  // Assume you have a function to free AST nodes
        cell->formula = NULL;
    }

    // Parse formula into AST (implement this parser)
    cell->formula = parse_formula(formula_str);
    if (!cell->formula) {
        return ERR_INVALID_FORMULA;
    }

    // Check for circular dependencies
    if (sheet_check_circular(sheet, (CellRef){row, col})) {
        free_ast(cell->formula);
        cell->formula = NULL;
        return ERR_CIRCULAR_REF;
    }

    // Trigger recalculation
    sheet_recalculate(sheet);
    return OK;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}



// Recursive formula evaluation
static int evaluate_ast(Spreadsheet *sheet, ASTNode *node, CalcError *err) {
    if (*err != OK) return 0;
    
    switch (node->type) {
        case VAL_CONST: return node->value;
        case VAL_CELL: {
            if (node->cellref.row >= sheet->rows || node->cellref.col >= sheet->cols) {
                *err = ERR_INVALID_REF;
                return 0;
            }
            return sheet->cells[node->cellref.row][node->cellref.col].value;
        }
        case OP_ADD: 
            return evaluate_ast(sheet, node->left, err) + evaluate_ast(sheet, node->right, err);
        case OP_SUB:
            return evaluate_ast(sheet, node->left, err) - evaluate_ast(sheet, node->right, err);
        case OP_MUL:
            return evaluate_ast(sheet, node->left, err) * evaluate_ast(sheet, node->right, err);
        case OP_DIV: {
            int denom = evaluate_ast(sheet, node->right, err);
            if (denom == 0) {
                *err = ERR_DIV_ZERO;
                return 0;
            }
            return evaluate_ast(sheet, node->left, err) / denom;
        }
        // Handle functions (MIN, MAX, etc.)
        default: {
            *err = ERR_INVALID_FORMULA;
            return 0;
        }
    }
}

// Trigger recalculation of dependent cells
void sheet_recalculate(Spreadsheet *sheet) {
    // Implement recalculation logic here
    // For now, just a placeholder
    for (int i = 0; i < sheet->rows; i++) {
        for (int j = 0; j < sheet->cols; j++) {
            Cell *cell = &sheet->cells[i][j];
            if (cell->formula) {
                CalcError err = OK;
                int new_val = evaluate_ast(sheet, cell->formula, &err);
                if (err == OK) cell->value = new_val;
            }
        }
    }
}

static Token current_token;
static const char *formula_ptr;

// Helper: Get next token
static void next_token() {
    while (isspace(*formula_ptr)) formula_ptr++;
    
    current_token.start = formula_ptr;
    current_token.length = 1;

    if (*formula_ptr == '\0') {
        current_token.type = TOK_EOF;
        return;
    }

    if (isdigit(*formula_ptr)) {
        current_token.type = TOK_NUMBER;
        current_token.num_value = strtol(formula_ptr, (char**)&formula_ptr, 10);
        current_token.length = formula_ptr - current_token.start;
        return;
    }

    if (isalpha(*formula_ptr)) {
        // Check for cell reference
        char col_str[4] = {0};
        int i = 0;
        
        while (isalpha(*formula_ptr) && i < 3) {
            col_str[i++] = *formula_ptr++;
        }
        
        if (isdigit(*formula_ptr)) {
            current_token.type = TOK_CELL;
            parse_cell_ref(current_token.start, &current_token.cell_value);
            current_token.length = formula_ptr - current_token.start;
            return;
        }
        
        // Handle functions later
    }

    switch (*formula_ptr) {
        case '+': current_token.type = TOK_PLUS; break;
        case '-': current_token.type = TOK_MINUS; break;
        case '*': current_token.type = TOK_MUL; break;
        case '/': current_token.type = TOK_DIV; break;
        case '(': current_token.type = TOK_LPAREN; break;
        case ')': current_token.type = TOK_RPAREN; break;
        default: current_token.type = TOK_INVALID; break;
    }

    formula_ptr++;
}

// Recursive descent parser functions
static ASTNode *parse_expression();
static ASTNode *parse_term();
static ASTNode *parse_factor();

ASTNode *parse_formula(const char *formula_str) {
    formula_ptr = formula_str;
    next_token(); // Initialize first token
    
    ASTNode *node = parse_expression();
    if (current_token.type != TOK_EOF) {
        free_ast(node);
        return NULL;
    }
    return node;
}

static ASTNode *parse_expression() {
    ASTNode *node = parse_term();
    
    while (current_token.type == TOK_PLUS || current_token.type == TOK_MINUS) {
        TokenType op = current_token.type;
        next_token();
        ASTNode *right = parse_term();
        node = create_binary_op(op, node, right);
    }
    
    return node;
}

static ASTNode *parse_term() {
    ASTNode *node = parse_factor();
    
    while (current_token.type == TOK_MUL || current_token.type == TOK_DIV) {
        TokenType op = current_token.type;
        next_token();
        ASTNode *right = parse_factor();
        node = create_binary_op(op, node, right);
    }
    
    return node;
}

static ASTNode *parse_factor() {
    ASTNode *node = NULL;
    
    switch (current_token.type) {
        case TOK_NUMBER:
            node = create_number_node(current_token.num_value);
            next_token();
            break;
            
        case TOK_CELL:
            node = create_cell_node(current_token.cell_value);
            next_token();
            break;
            
        case TOK_LPAREN:
            next_token();
            node = parse_expression();
            if (current_token.type != TOK_RPAREN) {
                free_ast(node);
                return NULL;
            }
            next_token();
            break;
            
        default:
            return NULL;
    }
    
    return node;
}

// AST node creation helpers
static ASTNode *create_number_node(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = VAL_CONST;
    node->value = value;
    node->left = node->right = NULL;
    return node;
}

static ASTNode *create_cell_node(CellRef cellref) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = VAL_CELL;
    node->cellref = cellref;
    node->left = node->right = NULL;
    return node;
}

static ASTNode *create_binary_op(TokenType op_type, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    
    switch (op_type) {
        case TOK_PLUS: node->type = OP_ADD; break;
        case TOK_MINUS: node->type = OP_SUB; break;
        case TOK_MUL: node->type = OP_MUL; break;
        case TOK_DIV: node->type = OP_DIV; break;
        default: free(node); return NULL;
    }
    
    node->left = left;
    node->right = right;
    return node;
}
