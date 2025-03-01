#include "../Declarations/ds.h"
#include "../Declarations/frontend.h"


// Helper function to compare pairs internally
static short compare_pairs(Pair a, Pair b) {
    if (a.i != b.i) {
        return a.i - b.i;
    }
    return a.j - b.j;
}

void vector_init(Vector* vector) {
    vector->size = 0;
    vector->capacity = 4;
    vector->data = (Pair*)malloc(vector->capacity * sizeof(Pair));
    // vector->sheet = sheet;
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

Pair* vector_iterator_next(VectorIterator* iterator) {
    if (!vector_iterator_has_next(iterator)) return NULL;
    return &(iterator->vector->data[iterator->index++]);
    // return &(iterator->vector->sheet->cells[pos.i][pos.j]);
}


void queue_init(Queue* queue, size_t capacity) {
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->data = (Pair*)malloc(queue->capacity * sizeof(Pair));
    // queue->sheet = sheet;
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

Pair* queue_dequeue(Queue* queue) {
    if (queue_is_empty(queue)) return NULL;
    Pair* pos = &(queue->data[queue->front]);
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return pos;
    // return &(queue->sheet->cells[pos.i][pos.j]);
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

Pair* queue_iterator_next(QueueIterator* iterator) {
    if (!queue_iterator_has_next(iterator)) return NULL;
    Pair* value = &(iterator->queue->data[iterator->index]);
    iterator->index = (iterator->index + 1) % iterator->queue->capacity;
    return value;
}


void stack_init(Stack* stack) {
    stack->size = 0;
    stack->capacity = 4;
    stack->data = (Pair*)malloc(stack->capacity * sizeof(Pair));
    // stack->sheet = sheet;
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

Pair* stack_pop(Stack* stack) {
    if (stack->size == 0) return NULL;
    stack->size--;
    return &(stack->data[stack->size]);
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

Pair* stack_iterator_next(StackIterator* iterator) {
    if (!stack_iterator_has_next(iterator)) return NULL;
    iterator->index--;
    return &(iterator->stack->data[iterator->index]);
}



// // AVL Node structure - direct without Set wrapper
// typedef struct AVLNode {
//     Pair pair;
//     struct AVLNode* left;
//     struct AVLNode* right;
//     unsigned char height;
// } AVLNode;




// Get height with null check
static inline unsigned char height(AVLNode* node) {
    return node ? node->height : 0;
}
// Get size with null check
// static inline size_t size(AVLNode* node) {
//     return node ? node->size : 0;
// }
// Get balance factor
static inline int get_balance(AVLNode* node) {
    return node ? height(node->left) - height(node->right) : 0;
}
// Update height and size of a node
static inline void update_node(AVLNode* node) {
    unsigned char h_left = height(node->left);
    unsigned char h_right = height(node->right);
    node->height = 1 + (h_left > h_right ? h_left : h_right);
}
// Create a new AVL node
AVLNode* avl_create_node(short row, short col) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) return NULL;
    
    node->pair.i = row;
    node->pair.j = col;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}
// Right rotation
static AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    update_node(y);
    update_node(x);
    
    return x;
}
// Left rotation
static AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    update_node(x);
    update_node(y);
    
    return y;
}
// AVL tree node insertion - returns new root
AVLNode* avl_insert(AVLNode* root, short row, short col) {
    if (!root)
        return avl_create_node(row, col);

    Pair new_pair = {row, col};
    short cmp = compare_pairs(new_pair, root->pair);
    
    if (cmp < 0)
        root->left = avl_insert(root->left, row, col);
    else if (cmp > 0)
        root->right = avl_insert(root->right, row, col);
    else
        return root; // No duplicates
    
    update_node(root);
    
    int balance = get_balance(root);
    
    // Left Left Case
    if (balance > 1 && compare_pairs(new_pair, root->left->pair) < 0)
        return right_rotate(root);
    
    // Right Right Case
    if (balance < -1 && compare_pairs(new_pair, root->right->pair) > 0)
        return left_rotate(root);
    
    // Left Right Case
    if (balance > 1 && compare_pairs(new_pair, root->left->pair) > 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }
    
    // Right Left Case
    if (balance < -1 && compare_pairs(new_pair, root->right->pair) < 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }
    
    return root;
}
// Find a pair in the AVL tree
AVLNode* find_node(AVLNode* root, short row, short col) {
    if (!root) return NULL;
    
    Pair search_pair = {row, col};
    short cmp = compare_pairs(search_pair, root->pair);
    
    if (cmp < 0)
        return find_node(root->left, row, col);
    else if (cmp > 0)
        return find_node(root->right, row, col);
    else
        return root;
}
// Find a pair - returns pointer to the pair or NULL if not found
Pair* avl_find(AVLNode* root, short row, short col) {
    AVLNode* node = find_node(root, row, col);
    return node ? &(node->pair) : NULL;
}
// Find minimum value node
static AVLNode* min_value_node(AVLNode* node) {
    AVLNode* current = node;
    while (current->left)
        current = current->left;
    return current;
}
// Remove a node from AVL tree
AVLNode* avl_remove(AVLNode* root, short row, short col) {
    if (!root) return NULL;
    
    Pair remove_pair = {row, col};
    short cmp = compare_pairs(remove_pair, root->pair);
    
    if (cmp < 0)
        root->left = avl_remove(root->left, row, col);
    else if (cmp > 0)
        root->right = avl_remove(root->right, row, col);
    else {
        // Node with only one child or no child
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            
            if (!temp) {
                // No child case
                temp = root;
                root = NULL;
            } else {
                // One child case
                *root = *temp; // Copy contents
            }
            
            free(temp);
        } else {
            // Node with two children
            AVLNode* temp = min_value_node(root->right);
            root->pair = temp->pair;
            root->right = avl_remove(root->right, temp->pair.i, temp->pair.j);
        }
    }
    
    if (!root) return NULL;
    
    update_node(root);
    
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
// Free the entire AVL tree
void avl_free(AVLNode* root) {
    if (root) {
        avl_free(root->left);
        avl_free(root->right);
        free(root);
        root = NULL;
    }
}








// void collect_traverse_avl_tree(AVLNode* node, AVLNode*** adjList, AVLNode** visited, Vector* sorted, Spreadsheet* sheet) {
//     if (node == NULL) return;

//     // In-order traversal: left, current, right
//     collect_traverse_avl_tree(node->left, adjList, visited, sorted, sheet);

//     // Process the current node
//     Pair p = node->pair;

//     // Only process if not already visited
//     if (avl_find(*visited, p.i, p.j) == NULL) {
//         // Process this cell recursively
//         Cell* dep_cell = &(sheet->cells[p.i][p.j]);
//         collect_traverse_topo(dep_cell, adjList, visited, sorted, sheet);
//     }

//     // Continue with right subtree
//     collect_traverse_avl_tree(node->right, adjList, visited, sorted, sheet);
// }

// void collect_traverse_topo(Cell* cell, AVLNode*** adjList, AVLNode** visited, Vector* sorted, Spreadsheet* sheet) {
//     if(cell == NULL) return;
//     // Mark current cell as visited
//     *visited = avl_insert(*visited, cell->row, cell->col);

//     // Use recursive traversal instead of iterators to process adjacency list
//     if (cell->topo_order > 0)
//     {
//         collect_traverse_avl_tree((*adjList)[cell->topo_order], adjList, visited, sorted, sheet);
//     }

//     // Add current cell to sorted list after processing all dependencies
//     vector_push_back(sorted, cell->row, cell->col);
// }




// void topological_sort_util(Cell* cell, Set* adjList, Set* visited, Vector* sorted) {
//     // Mark current cell as visited
//     set_add(visited, cell->row, cell->col);
    
//     // Process all adjacent cells (dependencies)
//     SetIterator it;
//     set_iterator_init(&it, &adjList[cell->topo_order]);
    
//     while (set_iterator_has_next(&it)) {
//         Cell* adj_cell = set_iterator_next(&it);
//         // Only process if not already visited
//         if (set_find(visited, adj_cell->row, adj_cell->col) == NULL) {
//             topological_sort_util(adj_cell, adjList, visited, sorted);
//         }
//     }
//     set_iterator_free(&it);

//     // Add current cell to sorted list after processing all dependencies
//     vector_push_back(sorted, cell->row, cell->col);
// }



void topologic_util(Cell* currcell, Vector* adjList, char* visited, Vector* sorted, Spreadsheet* sheet) {
    // Mark current cell as visited
    visited[currcell->topo_order] = 1;
    
    // Process all adjacent cells (dependencies)
    VectorIterator it;
    vector_iterator_init(&it, &adjList[currcell->topo_order]);
    while(vector_iterator_has_next(&it)) {
        Pair* adjcell = vector_iterator_next(&it);
        // Only process if not already visited
        if (visited[sheet->cells[adjcell->i][adjcell->j].topo_order] == 0) {
            topologic_util(&sheet->cells[adjcell->i][adjcell->j], adjList, visited, sorted, sheet);
        }
    }
    // Add current cell to sorted list after processing all dependencies
    vector_push_back(sorted, currcell->row, currcell->col);
}


void topological_sort(Vector* adjList, int numVertices, Pair** cell_map, Vector* result, Spreadsheet* sheet) {
    // Initialize visited set to track processed cells
    char *visited = (char*)malloc((numVertices+1) *sizeof(char));
    if (!visited) {
        fprintf(stderr, "Memory allocation failed for visited array\n");
        exit(1);
    }
    for (int i = 1; i <= numVertices; i++)
    {
        visited[i] = 0;
    }
    

    // Initialize result vector
    Vector sorted;
    vector_init(&sorted);

    // Process all vertices
    for (int index = 1; index <= numVertices; index++) {
        // The cell that has this topo_order
        Cell* current = &sheet->cells[(*cell_map)[index].i][(*cell_map)[index].j];
        // If it hasn't been visited, process it
        if (current != NULL && visited[current->topo_order] == 0) {
            topologic_util(current, adjList, visited, &sorted, sheet);
        }
    }

    // Create result vector for topo order
    vector_init(result);
    
    // TODO: REMOVE THIS Reverse the order (as DFS gives reverse topological sort)
    for (size_t i = 0; i < sorted.size; i++) {
        vector_push_back(result, sorted.data[i].i, sorted.data[i].j);
    }

    // Clean up
    free(visited);
    vector_free(&sorted);
    visited = NULL;
}

void create_cell(short row, short col, Cell* cell) {
    cell->row = row;
    cell->col = col;
    cell->topo_order = -1;
    cell->type = 'C';
    cell->value = 0;
    cell->cell_state = 'N';
    cell->dependents = NULL;
    // cell->dependencies = NULL;
    cell->has_error = false;
    cell->is_sleep = false;
}

void free_cell(Cell* cell) {
    // if(cell->dependencies != NULL) {
    //     set_free(cell->dependencies);
    //     free(cell->dependencies);
    //     cell->dependencies = NULL;
    // };
    if(cell->dependents != NULL) {
        avl_free(cell->dependents);
        cell->dependents = NULL;
    }
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


    Spreadsheet* sheet = (Spreadsheet*)malloc(sizeof(Spreadsheet));
    if (!sheet) {
        fprintf(stderr, "Memory allocation failed for spreadsheet\n");
        exit(1);
    }
    sheet->totalRows = rows;
    sheet->totalCols = cols;


    sheet->scroll_row = 0;
    sheet->scroll_col = 0;
    sheet->output_enabled = 1;

    sheet->last_status = STATUS_OK;

    sheet->cells = (Cell**)malloc(rows* sizeof(Cell*));
    for (int i = 0; i < rows; i++) {
        sheet->cells[i] = (Cell*)malloc(cols * sizeof(Cell));
        for (int j = 0; j < cols; j++) 
        {
            create_cell(i, j, &sheet->cells[i][j]);
        }
    }
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