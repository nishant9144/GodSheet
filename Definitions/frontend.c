#include "../Declarations/ds.h"
#include "../Declarations/frontend.h"
#include "../Declarations/parser.h" // Add this line to include the declaration of process_command

int output_enabled = 1; // Declared output_enabled variable
static struct termios original_term;

/* Terminal configuration */
// void configure_terminal() {
//     struct termios new_term;
//     tcgetattr(STDIN_FILENO, &original_term);
//     new_term = original_term;
//     new_term.c_lflag &= ~(ICANON | ECHO);
//     tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
// }
void configure_terminal() {
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;

    // new_term.c_lflag &= ~(ICANON);  // Disable line buffering
    new_term.c_lflag |= ECHO;       // Enable character display
    new_term.c_cc[VMIN] = 1;        // Process input immediately
    new_term.c_cc[VTIME] = 0;       // No timeout

    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}



void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

/* Convert column index to Excel-style label */
static void get_col_label(int col, char* buffer) {
    int len = 0;
    do {
        buffer[len++] = 'A' + (col % 26);
        col = col / 26 - 1;
    } while (col >= 0 && len < 3);
    
    // Reverse the string
    for(int i = 0; i < len/2; i++) {
        char tmp = buffer[i];
        buffer[i] = buffer[len-1-i];
        buffer[len-1-i] = tmp;
    }
    buffer[len] = '\0';
}


/* Display 10x10 viewport */
void display_viewport(Spreadsheet *sheet) {
    if (!output_enabled) return;
    printf(CLEAR_SCREEN);
    
    // Print column headers
    printf("    ");
    for(int col = sheet->scroll_col; 
        col < sheet->scroll_col + VIEWPORT_COLS && col < sheet->totalCols; 
        col++) {
        char col_buf[4];
        get_col_label(col, col_buf);
        printf("%-*s", CELL_WIDTH, col_buf);
    }
    printf("\n");

    // Print cells
    for(int row = sheet->scroll_row;
        row < sheet->scroll_row + VIEWPORT_ROWS && row < sheet->totalRows;
        row++) {
        printf("%3d ", row+1);
        for(int col = sheet->scroll_col;
            col < sheet->scroll_col + VIEWPORT_COLS && col < sheet->totalCols;
            col++) {
            Cell* cell = &sheet->cells[row][col];
            printf("%-*d", CELL_WIDTH, cell->value);
        }
        printf("\n");
    }

    
}

/* Handle scroll commands */
// void handle_scroll(Spreadsheet *sheet, char direction) {
//     switch(direction) {
//         case 'w':  // Up
//             sheet->scroll_row = (sheet->scroll_row >= 10) 
//                               ? sheet->scroll_row - 10 
//                               : 0;
//             break;
//         case 's':  // Down
//             if(sheet->scroll_row + 10 < sheet->totalRows - VIEWPORT_ROWS)
//                 sheet->scroll_row += 10;
//             break;
//         case 'a':  // Left
//             sheet->scroll_col = (sheet->scroll_col >= 10)
//                               ? sheet->scroll_col - 10
//                               : 0;
//             break;
//         case 'd':  // Right
//             if(sheet->scroll_col + 10 < sheet->totalCols - VIEWPORT_COLS)
//                 sheet->scroll_col += 10;
//             break;
//     }
// }
void handle_scroll(Spreadsheet *sheet, char direction) {
    printf("Scroll command received: %c\n", direction); // Debug print

    switch(direction) {
        case 'w':  // Up
            if (sheet->scroll_row >= 10) {
                sheet->scroll_row -= 10;
            } else {
                sheet->scroll_row = 0;
            }
            break;
        case 's':  // Down
            if(sheet->scroll_row + 10 < sheet->totalRows - VIEWPORT_ROWS) {
                sheet->scroll_row += 10;
            }
            break;
        case 'a':  // Left
            if (sheet->scroll_col >= 10) {
                sheet->scroll_col -= 10;
            } else {
                sheet->scroll_col = 0;
            }
            break;
        case 'd':  // Right
            if(sheet->scroll_col + 10 < sheet->totalCols - VIEWPORT_COLS) {
                sheet->scroll_col += 10;
            }
            break;
    }

    printf("New Scroll Position -> Row: %d, Col: %d\n", sheet->scroll_row, sheet->scroll_col);
}



void scroll_to(Spreadsheet *sheet, int row, int col) {
    // Ensure row and col are within valid bounds
    if (row < 0) row = 0;
    if (col < 0) col = 0;
    if (row >= sheet->totalRows) row = sheet->totalRows - 1;
    if (col >= sheet->totalCols) col = sheet->totalCols - 1;

    // Adjust scroll_row to ensure the target row is visible
    if (row < sheet->scroll_row) {
        sheet->scroll_row = row;  // Move viewport up
    } else if (row >= sheet->scroll_row + VIEWPORT_ROWS) {
        sheet->scroll_row = row - VIEWPORT_ROWS + 1;  // Move viewport down
    }

    // Adjust scroll_col to ensure the target column is visible
    if (col < sheet->scroll_col) {
        sheet->scroll_col = col;  // Move viewport left
    } else if (col >= sheet->scroll_col + VIEWPORT_COLS) {
        sheet->scroll_col = col - VIEWPORT_COLS + 1;  // Move viewport right
    }
}


/* Main UI loop */
void run_ui(Spreadsheet *sheet) {
    struct timeval start_time, end_time;
    char input[100];  // Fixed buffer size for input
    
    while(1) {
        gettimeofday(&start_time, NULL);
        
        // Display current view
        display_viewport(sheet);
        
        // Calculate elapsed time since last command
        double elapsed = (start_time.tv_sec - sheet->last_cmd_time.tv_sec) +
                 (start_time.tv_usec - sheet->last_cmd_time.tv_usec) / 1000000.0;

        // Avoid printing a huge number if uninitialized
        if (elapsed > 10000 || elapsed < 0) {
            elapsed = 0.0;
        }
        
        // Show prompt with status
        printf("[%.1f] (%s) > ", elapsed, 
               sheet->last_status == STATUS_OK ? "ok" : "error");
        fflush(stdout);
        
        // Get input
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';  // Remove newline
        printf("Input received: %s (ASCII: %d)\n", input, input[0]); // Debugging

        
        // Handle commands
        // Handle commands
        if (strlen(input) == 0)
            continue;

        // Check for quit command
        if (strcmp(input, "q") == 0){
            break;
        }

        // First check for full-string commands
        if (strcmp(input, "disable_output") == 0) {
            output_enabled = 0;
            sheet->last_status = STATUS_OK;
            continue;
        }

        if (strcmp(input, "enable_output") == 0) {
            output_enabled = 1;
            sheet->last_status = STATUS_OK;
            display_viewport(sheet); // Show UI again after enabling output
            continue;
        }

        
        if (strncmp(input, "goto ", 5) == 0) {
            char cell_ref[10];
            sscanf(input + 5, "%s", cell_ref);
            int col = 0, i = 0;
            while (cell_ref[i] && isalpha(cell_ref[i])) {
                col = col * 26 + (toupper(cell_ref[i]) - 'A' + 1);
                i++;
            }
            col = col - 1;  // Convert to 0-based index
            int row = atoi(cell_ref + i) - 1;  // Convert to 0-based index
            scroll_to(sheet, row, col);
            sheet->last_status = STATUS_OK;
            if (output_enabled)
                display_viewport(sheet);
            continue;
        }

        
        // If input is a single character and that character is one of "wasd", treat it as a scroll command
        if (strlen(input) == 1 && strchr("wasd", input[0]) != NULL) {
            handle_scroll(sheet, input[0]);
            sheet->last_status = STATUS_OK;
            if (output_enabled)
                display_viewport(sheet);
            continue;
        }

        // Otherwise, process formula or other commands
        process_command(sheet, input);
        
        // Update last command time
        gettimeofday(&end_time, NULL);
        sheet->last_cmd_time = end_time;

        // Calculate and print the time taken to process the command
        double processing_time = (end_time.tv_sec - start_time.tv_sec) +
                                 (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        printf("Processing time: %.6f seconds\n", processing_time);
    }
}