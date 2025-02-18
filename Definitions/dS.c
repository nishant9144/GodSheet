#include "../Declarations/ds.h"
#include "../Declarations/frontend.h"


// Helper function to compare pairs internally
static short compare_pairs(Pair a, Pair b) {
    if (a.i != b.i) {
        return a.i - b.i;
    }
    return a.j - b.j;
}

void vector_init(Vector* vector, Spreadsheet* sheet) {
    vector->size = 0;
    vector->capacity = 4;
    vector->data = (Pair*)malloc(vector->capacity * sizeof(Pair));
    vector->sheet = sheet;
}

void vector_push_back(Vector* vector, short row, short col) {
    if (vector->size == vector->capacity) {
        vector->capacity *= 2;
        vector->data = (Pair*)realloc(vector->data, vector->capacity * sizeof(Pair));
    }
    vector->data[vector->size].i = row;
    vector->data[vector->size].j = col;
    vector->size++;
}


void vector_free(Vector* vector) {
    if (vector->data) {
        free(vector->data);
        vector->data = NULL;
    }
    vector->size = 0;
    vector->capacity = 0;
}


void vector_iterator_init(VectorIterator* iterator, Vector* vector) {
    iterator->vector = vector;
    iterator->index = 0;
}

bool vector_iterator_has_next(VectorIterator* iterator) {
    return iterator->index < iterator->vector->size;
}

Cell* vector_iterator_next(VectorIterator* iterator) {
    if (!vector_iterator_has_next(iterator)) return NULL;
    Pair pos = iterator->vector->data[iterator->index++];
    return &(iterator->vector->sheet->cells[pos.i][pos.j]);
}


void queue_init(Queue* queue, size_t capacity, Spreadsheet* sheet) {
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->data = (Pair*)malloc(queue->capacity * sizeof(Pair));
    queue->sheet = sheet;
}

bool queue_is_full(Queue* queue) {
    return (queue->size == queue->capacity);
}

bool queue_is_empty(Queue* queue) {
    return (queue->size == 0);
}

void queue_enqueue(Queue* queue, short row, short col) {
    if (queue_is_full(queue)) return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->data[queue->rear].i = row;
    queue->data[queue->rear].j = col;
    queue->size++;
}

Cell* queue_dequeue(Queue* queue) {
    if (queue_is_empty(queue)) return NULL;
    Pair pos = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return &(queue->sheet->cells[pos.i][pos.j]);
}

void queue_free(Queue* queue) {
    if (queue->data) {
        free(queue->data);
        queue->data = NULL;
    }
    queue->size = 0;
}


void queue_iterator_init(QueueIterator* iterator, Queue* queue) {
    iterator->queue = queue;
    iterator->index = queue->front;
}

bool queue_iterator_has_next(QueueIterator* iterator) {
    return iterator->index != (iterator->queue->rear + 1) % iterator->queue->capacity;
}

Cell* queue_iterator_next(QueueIterator* iterator) {
    if (!queue_iterator_has_next(iterator)) return NULL;
    Cell* value = &(iterator->queue->sheet->cells[iterator->queue->data[iterator->index].i]
                                                [iterator->queue->data[iterator->index].j]);
    iterator->index = (iterator->index + 1) % iterator->queue->capacity;
    return value;
}


void stack_init(Stack* stack, Spreadsheet* sheet) {
    stack->size = 0;
    stack->capacity = 4;
    stack->data = (Pair*)malloc(stack->capacity * sizeof(Pair));
    stack->sheet = sheet;
}

void stack_push(Stack* stack, short row, short col) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->data = (Pair*)realloc(stack->data, stack->capacity * sizeof(Pair));
    }
    stack->data[stack->size].i = row;
    stack->data[stack->size].j = col;
    stack->size++;
}

Cell* stack_pop(Stack* stack) {
    if (stack->size == 0) return NULL;
    stack->size--;
    return &(stack->sheet->cells[stack->data[stack->size].i][stack->data[stack->size].j]);
}

void stack_free(Stack* stack) {
    if (stack->data) {
        free(stack->data);
        stack->data = NULL;
    }
    stack->size = 0;
    stack->capacity = 0;
}

void stack_iterator_init(StackIterator* iterator, Stack* stack) {
    iterator->stack = stack;
    iterator->index = stack->size;
}

bool stack_iterator_has_next(StackIterator* iterator) {
    return iterator->index > 0;
}

Cell* stack_iterator_next(StackIterator* iterator) {
    if (!stack_iterator_has_next(iterator)) return NULL;
    iterator->index--;
    return &(iterator->stack->sheet->cells[iterator->stack->data[iterator->index].i]
                                        [iterator->stack->data[iterator->index].j]);
}


static int height(AVLNode* node) {
    return node ? node->height : 0;
}

static int get_balance(AVLNode* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

static AVLNode* create_node(short row, short col) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->pair.i = row;
    node->pair.j = col;
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


void set_init(Set* set, Spreadsheet* sheet) {
    set->root = NULL;
    set->sheet = sheet;
}

static AVLNode* insert(AVLNode* node, short row, short col) {
    if (!node)
        return create_node(row, col);

    Pair new_pair = {row, col};
    short cmp = compare_pairs(new_pair, node->pair);
    
    if (cmp < 0)
        node->left = insert(node->left, row, col);
    else if (cmp > 0)
        node->right = insert(node->right, row, col);
    else
        return node;

    node->height = 1 + (height(node->left) > height(node->right) ? 
                       height(node->left) : height(node->right));

    int balance = get_balance(node);

    // Left Left Case
    if (balance > 1 && compare_pairs(new_pair, node->left->pair) < 0)
        return right_rotate(node);

    // Right Right Case
    if (balance < -1 && compare_pairs(new_pair, node->right->pair) > 0)
        return left_rotate(node);

    // Left Right Case
    if (balance > 1 && compare_pairs(new_pair, node->left->pair) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // Right Left Case
    if (balance < -1 && compare_pairs(new_pair, node->right->pair) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

void set_add(Set* set, short row, short col) {
    set->root = insert(set->root, row, col);
}

static AVLNode* find(AVLNode* node, short row, short col) {
    if (!node) return NULL;

    Pair search_pair = {row, col};
    short cmp = compare_pairs(search_pair, node->pair);
    
    if (cmp < 0)
        return find(node->left, row, col);
    else if (cmp > 0)
        return find(node->right, row, col);
    else
        return node;
}

Cell* set_find(Set* set, short row, short col) {
    AVLNode* result = find(set->root, row, col);
    if (!result) return NULL;
    return &(set->sheet->cells[row][col]);
}

static AVLNode* min_value_node(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

static AVLNode* remove_node(AVLNode* root, short row, short col) {
    if (!root) return root;

    Pair remove_pair = {row, col};
    short cmp = compare_pairs(remove_pair, root->pair);
    
    if (cmp < 0)
        root->left = remove_node(root->left, row, col);
    else if (cmp > 0)
        root->right = remove_node(root->right, row, col);
    else {
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            if (!temp) {
                temp = root;
                root = NULL;
            } else
                *root = *temp;
            free(temp);
        } else {
            AVLNode* temp = min_value_node(root->right);
            root->pair = temp->pair;
            root->right = remove_node(root->right, temp->pair.i, temp->pair.j);
        }
    }

    if (!root) return root;

    root->height = 1 + (height(root->left) > height(root->right) ? 
                       height(root->left) : height(root->right));
    int balance = get_balance(root);

    // Left Left Case
    if (balance > 1 && get_balance(root->left) >= 0)
        return right_rotate(root);

    // Left Right Case
    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    // Right Right Case
    if (balance < -1 && get_balance(root->right) <= 0)
        return left_rotate(root);

    // Right Left Case
    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

void set_remove(Set* set, short row, short col) {
    set->root = remove_node(set->root, row, col);
}

static void free_tree(AVLNode* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        free(node);
    }
}

void set_free(Set* set) {
    free_tree(set->root);
    set->root = NULL;
}

// Set Iterator implementation
void set_iterator_init(SetIterator* iterator, Set* set) {
    iterator->set = set;
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

bool set_iterator_has_next(SetIterator* iterator) {
    return iterator->top > 0;
}

Cell* set_iterator_next(SetIterator* iterator) {
    if (!set_iterator_has_next(iterator))
        return NULL;

    AVLNode* node = iterator->stack[--iterator->top];
    Cell* cell = &(iterator->set->sheet->cells[node->pair.i][node->pair.j]);

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
    set_add(visited, cell->row, cell->col);
    
    // Process all adjacent cells (dependencies)
    SetIterator it;
    set_iterator_init(&it, &adjList[cell->topo_order]);
    
    while (set_iterator_has_next(&it)) {
        Cell* adj_cell = set_iterator_next(&it);
        // Only process if not already visited
        if (set_find(visited, adj_cell->row, adj_cell->col) == NULL) {
            topological_sort_util(adj_cell, adjList, visited, sorted);
        }
    }
    set_iterator_free(&it);

    // Add current cell to sorted list after processing all dependencies
    vector_push_back(sorted, cell->row, cell->col);
}

void topological_sort(Set* adjList, int numVertices, Cell** cell_map, Vector* result, Spreadsheet* sheet) {
    // Initialize visited set to track processed cells
    Set visited;
    set_init(&visited, sheet);

    // Initialize result vector
    Vector sorted;
    vector_init(&sorted, sheet);

    // Process all vertices
    for (int i = 1; i <= numVertices; i++) {
        // The cell that has this topo_order
        Cell* current = cell_map[i];
        // If it hasn't been visited, process it
        if (current != NULL && set_find(&visited, current->row, current->col) == NULL) {
            topological_sort_util(current, adjList, &visited, &sorted);
        }
    }

    // Create result vector for topo order
    vector_init(result, sheet);
    
    // Reverse the order (as DFS gives reverse topological sort)
    for (size_t i = 0; i < sorted.size; i++) {
        vector_push_back(result, sorted.data[i].i, sorted.data[i].j);
    }

    // Clean up
    set_free(&visited);
    vector_free(&sorted);

}

Cell* create_cell(short row, short col) {
    Cell* cell = (Cell*)malloc(sizeof(Cell));
    cell->row = row;
    cell->col = col;
    cell->topo_order = -1;
    cell->type = 'C';
    cell->value = 0; // Use 0 as initial value
    cell->dependents = NULL;
    cell->dependencies = NULL;
    cell->has_error = false;
    cell->is_sleep = false;
    return cell;
}

void free_cell(Cell* cell) {
    if(cell->dependencies != NULL) set_free(cell->dependencies);
    if(cell->dependents != NULL) set_free(cell->dependents);
}

short colNameToNumber(const char *colName) {
    short result = 0;
    while (*colName) {
        if (!isalpha(*colName)) return -1; // Invalid character check
        result = result * 26 + (toupper(*colName) - 'A' + 1);
        colName++;
    }
    result--; // 0 based indexing
    return result;
}

void colNumberToName(short colNumber, char *colName) { // 0 based argument
    if (colNumber < 0 || colNumber > 18277) { // Limit for "ZZZ"
        strcpy(colName, "\0");
        return;
    }
    colNumber++;
    short index = 0;
    char temp[4];
    while (colNumber > 0) {
        colNumber--;
        temp[index++] = 'A' + (colNumber % 26);
        colNumber /= 26;
    }
    temp[index] = '\0';
    
    // Reverse the result to get correct column name
    short len = strlen(temp);
    for (int i = 0; i < len; i++) {
        colName[i] = temp[len - 1 - i];
    }
    colName[len] = '\0';
    return;
}

Spreadsheet* create_spreadsheet(short rows, short cols){
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