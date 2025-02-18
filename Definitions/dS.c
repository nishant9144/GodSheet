// dS.c
#include "../Declarations/ds.h"
#include "../Declarations/frontend.h"

// Helper function to compare cells based on (row, col) tuple
int compare_cells_position(Cell* a, Cell* b) {
    // First compare rows
    if (a->row != b->row) {
        return a->row - b->row;
    }
    return strcmp(a->col, b->col);
}

// Vector implementation for Cells
void vector_init(Vector* vector) {
    vector->size = 0;
    vector->capacity = 4;
    vector->data = (Cell**)malloc(vector->capacity * sizeof(Cell*));
}

void vector_push_back(Vector* vector, Cell* value) {
    if (vector->size == vector->capacity) {
        vector->capacity *= 2;
        vector->data = (Cell**)realloc(vector->data, vector->capacity * sizeof(Cell*));
    }
    vector->data[vector->size++] = value;
}

void vector_free(Vector* vector) {
    free(vector->data);
    vector->data = NULL;
}

// Vector iterator implementation
void vector_iterator_init(VectorIterator* iterator, Vector* vector) {
    iterator->vector = vector;
    iterator->index = 0;
}

int vector_iterator_has_next(VectorIterator* iterator) {
    return iterator->index < iterator->vector->size;
}

Cell* vector_iterator_next(VectorIterator* iterator) {
    return iterator->vector->data[iterator->index++];
}


// Queue implementation for Cells
void queue_init(Queue* queue, size_t capacity) {
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->data = (Cell**)malloc(queue->capacity * sizeof(Cell*));
}

int queue_is_full(Queue* queue) {
    return (queue->size == queue->capacity);
}

int queue_is_empty(Queue* queue) {
    return (queue->size == 0);
}

void queue_enqueue(Queue* queue, Cell* value) {
    if (queue_is_full(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    // Create a deep copy when enqueueing
    queue->data[queue->rear] = (value);
    queue->size++;
}

Cell* queue_dequeue(Queue* queue) {
    if (queue_is_empty(queue))
        return NULL;
    Cell* value = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return value;
}

void queue_free(Queue* queue) {
    // Free each cell copy
    // for (size_t i = 0; i < queue->capacity; i++) {
    //     if (queue->data[i]) {
    //         free_cell_copy(queue->data[i]);
    //     }
    // }
    free(queue->data);
    queue->data = NULL;
}

// Queue iterator implementation
void queue_iterator_init(QueueIterator* iterator, Queue* queue) {
    iterator->queue = queue;
    iterator->index = queue->front;
}

int queue_iterator_has_next(QueueIterator* iterator) {
    return iterator->index != (iterator->queue->rear + 1) % iterator->queue->capacity;
}

Cell* queue_iterator_next(QueueIterator* iterator) {
    Cell* value = iterator->queue->data[iterator->index];
    iterator->index = (iterator->index + 1) % iterator->queue->capacity;
    return value;
}

// Stack implementation for Cells
void stack_init(Stack* stack) {
    stack->size = 0;
    stack->capacity = 4;
    stack->data = (Cell**)malloc(stack->capacity * sizeof(Cell*));
}

void stack_push(Stack* stack, Cell* value) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->data = (Cell**)realloc(stack->data, stack->capacity * sizeof(Cell*));
    }
    // Create a deep copy when pushing
    stack->data[stack->size++] = (value);
}

Cell* stack_pop(Stack* stack) {
    if (stack->size == 0)
        return NULL;
    return stack->data[--stack->size];
}

void stack_free(Stack* stack) {
    // Free each cell copy
    // for (int i = 0; i < stack->size; i++) {
    //     free_cell_copy(stack->data[i]);
    // }
    free(stack->data);
    stack->data = NULL;
}

// Stack iterator implementation
void stack_iterator_init(StackIterator* iterator, Stack* stack) {
    iterator->stack = stack;
    iterator->index = stack->size;
}

int stack_iterator_has_next(StackIterator* iterator) {
    return iterator->index > 0;
}

Cell* stack_iterator_next(StackIterator* iterator) {
    return iterator->stack->data[--iterator->index];
}

// AVL Tree Node for Set implementation
static int height(AVLNode* node) {
    return node ? node->height : 0;
}

static int get_balance(AVLNode* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

static AVLNode* create_node(Cell* cell) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    // Create a deep copy when creating a new node
    node->cell = (cell);
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

static AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + (height(y->left) > height(y->right) ? 
                     height(y->left) : height(y->right));
    x->height = 1 + (height(x->left) > height(x->right) ? 
                     height(x->left) : height(x->right));
    return x;
}

static AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + (height(x->left) > height(x->right) ? 
                     height(x->left) : height(x->right));
    y->height = 1 + (height(y->left) > height(y->right) ? 
                     height(y->left) : height(y->right));
    return y;
}

// Set implementation using AVL Tree
void set_init(Set* set) {
    set->root = NULL;
}

static AVLNode* insert(AVLNode* node, Cell* cell) {
    if (!node)
        return create_node(cell);

    int cmp = compare_cells_position(cell, node->cell);
    if (cmp < 0)
        node->left = insert(node->left, cell);
    else if (cmp > 0)
        node->right = insert(node->right, cell);
    else
        return node; // No duplicate cells

    node->height = 1 + (height(node->left) > height(node->right) ? 
                       height(node->left) : height(node->right));

    int balance = get_balance(node);

    // Left Left Case
    if (balance > 1 && compare_cells_position(cell, node->left->cell) < 0)
        return right_rotate(node);

    // Right Right Case
    if (balance < -1 && compare_cells_position(cell, node->right->cell) > 0)
        return left_rotate(node);

    // Left Right Case
    if (balance > 1 && compare_cells_position(cell, node->left->cell) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // Right Left Case
    if (balance < -1 && compare_cells_position(cell, node->right->cell) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

void set_add(Set* set, Cell* cell) {
    set->root = insert(set->root, cell);
}

static AVLNode* find(AVLNode* node, Cell* cell) {
    if (!node)
        return NULL;

    int cmp = compare_cells_position(cell, node->cell);
    if (cmp < 0)
        return find(node->left, cell);
    else if (cmp > 0)
        return find(node->right, cell);
    else
        return node;
}

Cell* set_find(Set* set, Cell* cell) {
    AVLNode* result = find(set->root, cell);
    if (result == NULL)
        return NULL;
    return result->cell;
}


static AVLNode* min_value_node(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

static AVLNode* remove_node(AVLNode* root, Cell* cell) {
    if (root == NULL)
        return root;

    int cmp = compare_cells_position(cell, root->cell);
    if (cmp < 0)
        root->left = remove_node(root->left, cell);
    else if (cmp > 0)
        root->right = remove_node(root->right, cell);
    else {
        if ((root->left == NULL) || (root->right == NULL)) {
            AVLNode* temp = root->left ? root->left : root->right;
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else
                *root = *temp;
            free(temp);
            temp = NULL;
        } else {
            AVLNode* temp = min_value_node(root->right);
            root->cell = temp->cell;
            root->right = remove_node(root->right, temp->cell);
        }
    }

    if (root == NULL)
        return root;

    root->height = 1 + (height(root->left) > height(root->right) ? height(root->left) : height(root->right));
    int balance = get_balance(root);

    if (balance > 1 && get_balance(root->left) >= 0)
        return right_rotate(root);

    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    if (balance < -1 && get_balance(root->right) <= 0)
        return left_rotate(root);

    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

void set_remove(Set* set, Cell* cell) {
    set->root = remove_node(set->root, cell);
}


static void free_tree(AVLNode* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        // free_cell_copy(node->cell);  // Free the cell copy
        free(node);
        node = NULL;
    }
}

void set_free(Set* set) {
    free_tree(set->root);
}

// Set Iterator implementation
void set_iterator_init(SetIterator* iterator, Set* set) {
    iterator->capacity = 32; // Initial capacity
    iterator->stack = (AVLNode**)malloc(iterator->capacity * sizeof(AVLNode*));
    iterator->top = 0;

    // Push all left nodes onto stack for inorder traversal
    AVLNode* current = set->root;
    while (current) {
        if (iterator->top == iterator->capacity) {
            iterator->capacity *= 2;
            iterator->stack = (AVLNode**)realloc(iterator->stack, 
                                               iterator->capacity * sizeof(AVLNode*));
        }
        iterator->stack[iterator->top++] = current;
        current = current->left;
    }
}

int set_iterator_has_next(SetIterator* iterator) {
    return iterator->top > 0;
}

Cell* set_iterator_next(SetIterator* iterator) {
    if (!set_iterator_has_next(iterator))
        return NULL;

    AVLNode* node = iterator->stack[--iterator->top];
    Cell* cell = node->cell;

    // Push all left nodes of the right subtree
    AVLNode* current = node->right;
    while (current) {
        if (iterator->top == iterator->capacity) {
            iterator->capacity *= 2;
            iterator->stack = (AVLNode**)realloc(iterator->stack, 
                                               iterator->capacity * sizeof(AVLNode*));
        }
        iterator->stack[iterator->top++] = current;
        current = current->left;
    }

    return cell;
}

void set_iterator_free(SetIterator* iterator) {
    free(iterator->stack);
    iterator->stack = NULL;
}


void topological_sort_util(Cell* cell, Set* adjList, Set* visited, Vector* sorted) {
    // Mark current cell as visited
    set_add(visited, cell);
    
    // Process all adjacent cells (dependencies)
    SetIterator it;
    set_iterator_init(&it, &adjList[cell->topo_order]);
    
    while (set_iterator_has_next(&it)) {
        Cell* adj_cell = set_iterator_next(&it);
        // Only process if not already visited
        if (set_find(visited, adj_cell) == NULL) {
            topological_sort_util(adj_cell, adjList, visited, sorted);
        }
    }
    set_iterator_free(&it);

    // Add current cell to sorted list after processing all dependencies
    vector_push_back(sorted, cell);
}

Vector topological_sort(Set* adjList, int numVertices, Cell** cell_map) {
    // Initialize visited set to track processed cells
    Set visited;
    set_init(&visited);

    // Initialize result vector
    Vector sorted;
    vector_init(&sorted);

    // Process all vertices
    for (int i = 1; i <= numVertices; i++) {
        // The cell that has this topo_order
        Cell* current = cell_map[i];
        // If it hasn't been visited, process it
        if (current != NULL && set_find(&visited, current) == NULL) {
            topological_sort_util(current, adjList, &visited, &sorted);
        }
    }

    // Create result vector for topo order
    Vector result;
    vector_init(&result);
    
    // Reverse the order (as DFS gives reverse topological sort)
    for (size_t i = 0; i < sorted.size; i++) {
        vector_push_back(&result, sorted.data[i]);
    }

    // Clean up
    set_free(&visited);
    vector_free(&sorted);

    return result;
}

Cell* create_cell(int row, int col) {
    Cell* cell = (Cell*)malloc(sizeof(Cell));
    cell->row = row;
    cell->col = (char *)malloc(4 * sizeof(char)); 
    if(cell->col == NULL){fprintf(stderr, "Cell allocation failed. line 458\n"); exit(1);} 
    colNumberToName(col,cell->col);  // Make a copy of the column string
    cell->topo_order = -1;
    cell->type = TYPE_CONSTANT;
    cell->value = 0; // Use 0 as initial value
    cell->formula = NULL;
    cell->dependents = NULL;
    cell->dependencies = NULL;
    cell->has_error = false;
    // cell->error_msg = NULL;
    // cell->visited = false;
    // cell->in_stack = false;
    cell->is_sleep = false;
    return cell;
}

void free_cell(Cell* cell) {
    if(cell->col!=NULL){
        free(cell->col);
        cell->col = NULL;
    } 
    if(cell->formula != NULL){
        free(cell->formula);
        cell->formula = NULL;
    } 
    // if(cell->error_msg != NULL) free(cell->error_msg);
    if(cell->dependencies != NULL) set_free(cell->dependencies);
    if(cell->dependents != NULL) set_free(cell->dependents);
    // free(cell);
}

int colNameToNumber(const char *colName) {
    int result = 0;
    while (*colName) {
        if (!isalpha(*colName)) return -1; // Invalid character check
        result = result * 26 + (toupper(*colName) - 'A' + 1);
        colName++;
    }
    result--; // 0 based indexing
    return result;
}

void colNumberToName(int colNumber, char *colName) { // 0 based argument
    if (colNumber < 0 || colNumber > 18277) { // Limit for "ZZZ"
        strcpy(colName, "\0");
        return;
    }
    colNumber++;
    int index = 0;
    char temp[4];
    while (colNumber > 0) {
        colNumber--;
        temp[index++] = 'A' + (colNumber % 26);
        colNumber /= 26;
    }
    temp[index] = '\0';
    
    // Reverse the result to get correct column name
    int len = strlen(temp);
    for (int i = 0; i < len; i++) {
        colName[i] = temp[len - 1 - i];
    }
    colName[len] = '\0';
    return;
}

Spreadsheet* create_spreadsheet(int rows, int cols){
    printf("Inside create_spreadsheet: rows=%d, cols=%d\n", rows, cols); //debug statement


    Spreadsheet* sheet = (Spreadsheet*)malloc(sizeof(Spreadsheet));
    if (!sheet) {
        fprintf(stderr, "Memory allocation failed for spreadsheet\n");
        exit(1);
    }
    sheet->totalRows = rows;
    sheet->totalCols = cols;

    printf("After assigning values: totalRows=%d, totalCols=%d\n", 
        sheet->totalRows, sheet->totalCols);

    sheet->scroll_row = 0;
    sheet->scroll_col = 0;
    sheet->output_enabled = 1;
    // sheet->mode = VIEW_MODE;y
    sheet->last_status = STATUS_OK;

    sheet->cells = (Cell**)malloc(rows* sizeof(Cell*));
    for (int i = 0; i < rows; i++) {
        sheet->cells[i] = (Cell*)malloc(cols * sizeof(Cell));
        for (int j = 0; j < cols; j++) sheet->cells[i][j] = *create_cell(i, j);
    }
    printf("Final values before returning: totalRows=%d, totalCols=%d\n",
        sheet->totalRows, sheet->totalCols);
    return sheet;
}

void print_spreadsheet(Spreadsheet* sheet){
    printf("  ");
    char* colname = (char*)malloc(4 * sizeof(char));
    for(int i = 0; i < sheet->totalCols; i++){
        colNumberToName(i, colname);
        printf(" %s ", colname);
    }
    free(colname);
    colname = NULL;
    printf("\n");

    for (int i = 0; i < sheet->totalRows; i++) {
        printf("%d ", i+1);
        for (int j = 0; j < sheet->totalCols; j++) {
            Cell* cell = &sheet->cells[i][j];
            printf(" %d ", cell->value);
        }
        printf("\n");
    }
}

void free_spreadsheet(Spreadsheet* sheet){
    for (int i = 0; i < sheet->totalRows; i++) {
        for (int j = 0; j < sheet->totalCols; j++) {
            free_cell(&(sheet->cells[i][j]));
        }
        free(sheet->cells[i]);
        sheet->cells[i] = NULL;
    }
    free(sheet->cells);
    sheet->cells = NULL;
    free(sheet);
    sheet = NULL;
}