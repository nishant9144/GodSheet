#include "../Declarations/ds.h"
#include "../Declarations/frontend.h"

static struct termios original_term;

// Helper functions
void configure_terminal()
{
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void clear_screen()
{
    write(STDOUT_FILENO, CLEAR_SCREEN, sizeof(CLEAR_SCREEN));
}

void display_status_bar(Spreadsheet *sheet)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    double elapsed = ((now.tv_sec - sheet->last_cmd_time.tv_sec) +
                      (now.tv_usec - sheet->last_cmd_time.tv_usec)) /
                     1000000.0;

    const char *status_msg;
    switch (sheet->last_status)
    {
    case STATUS_OK:
        status_msg = "ok";
        break;
    case ERR_INVALID_CELL:
        status_msg = "Invalid cell";
        break;
    case ERR_CIRCULAR_REF:
        status_msg = "Circular ref";
        break;
    case ERR_DIV_ZERO:
        status_msg = "Div by zero";
        break;
    case ERR_INVALID_RANGE:
        status_msg = "Invalid range";
        break;
    case ERR_SYNTAX:
        status_msg = "Syntax error";
        break;
    default:
        status_msg = "Unknown error";
    }

    printf("[%.1f] (%s) > ", elapsed, status_msg);
    fflush(stdout);
}

void display_viewport(Spreadsheet *sheet)
{
    write(STDOUT_FILENO, CLEAR_SCREEN, sizeof(CLEAR_SCREEN));

    int end_row = sheet->scroll_row + VIEWPORT_ROWS;
    int end_col = sheet->scroll_col + VIEWPORT_COLS;
    end_row = end_row > sheet->totalRows ? sheet->totalRows : end_row;
    end_col = end_col > sheet->totalCols ? sheet->totalCols : end_col;

    // Display mode and navigation info
    char start_col_ref[4], end_col_ref[4];
    int num = sheet->scroll_col;
    int idx = 0;
    do
    {
        start_col_ref[idx++] = 'A' + (num % 26);
        num = num / 26 - 1;
    } while (num >= 0 && idx < 3);
    start_col_ref[idx] = '\0';

    num = end_col - 1;
    idx = 0;
    do
    {
        end_col_ref[idx++] = 'A' + (num % 26);
        num = num / 26 - 1;
    } while (num >= 0 && idx < 3);
    end_col_ref[idx] = '\0';

    printf("Mode: %s    Page: Row %d-%d, Col %s-%s\n",
           sheet->mode == VIEW_MODE ? "VIEW" : "EDIT",
           sheet->scroll_row + 1, end_row,
           start_col_ref, end_col_ref);

    // Display column headers
    printf("     ");
    for (int j = sheet->scroll_col; j < end_col; j++)
    {
        char col_ref[4];
        int num = j;
        int idx = 0;
        do
        {
            col_ref[idx++] = 'A' + (num % 26);
            num = num / 26 - 1;
        } while (num >= 0 && idx < 3);
        col_ref[idx] = '\0';
        printf("%-*s", CELL_WIDTH, col_ref);
    }
    printf("\n");

    // Print horizontal separator
    printf("    +-");
    for (int j = sheet->scroll_col; j < end_col; j++)
    {
        for (int k = 0; k < CELL_WIDTH; k++)
            printf("-");
        printf("-");
    }
    printf("\n");

    // Display Cells
    for (int i = sheet->scroll_row; i < end_row; i++)
    {
        printf("%4d ", i + 1);
        for (int j = sheet->scroll_col; j < end_col; j++)
        {
            Cell *cell = &sheet->cells[i][j];
            if (i == sheet->cursorRow && j == sheet->cursorCol)
            {
                printf("\x1b[47m\x1b[30m"); // Highlight
            }

            if (cell->formula != NULL)
            {
                printf("=%-*s", CELL_WIDTH, cell->formula);
            }
            else
            {
                printf("%-*d", CELL_WIDTH, cell->value);
            }

            if (i == sheet->cursorRow && j == sheet->cursorCol)
            {
                printf("\x1b[0m"); // Reset
            }
        }
        printf("\n");
    }

    // Print bottom border
    printf("    +-");
    for (int j = sheet->scroll_col; j < end_col; j++)
    {
        for (int k = 0; k < CELL_WIDTH; k++)
            printf("-");
        printf("-");
    }
    printf("\n");

    // Display controls
    printf("\nControls:\n");
    printf("Arrow keys: Move cursor    M: Switch Mode    Q: Quit\n");
    printf("Page Navigation: N: Next page    P: Previous page\n");
    printf("R: Next row page    C: Next column page\n");
    if (sheet->mode == EDIT_MODE)
    {
        printf("Enter: Edit cell    Esc: Cancel edit\n");
    }

    display_status_bar(sheet);
}

char readArrowKeys()
{
    char c = getchar();
    if (c == ESC)
    {
        c = getchar(); // Skip '['
        if (c == '[')
        {
            return getchar();
        }
        return c;
    }
    return c;
}

void handleNavigation(Spreadsheet *sheet, char key)
{
    switch (key)
    {
    case ARROW_UP:
        if (sheet->cursorRow > 0)
        {
            sheet->cursorRow--;
            // Scroll viewport if necessary
            if (sheet->cursorRow < sheet->scroll_row)
            {
                sheet->scroll_row = sheet->cursorRow;
            }
        }
        break;
    case ARROW_DOWN:
        if (sheet->cursorRow < sheet->totalRows - 1)
        {
            sheet->cursorRow++;
            // Scroll viewport if necessary
            if (sheet->cursorRow >= sheet->scroll_row + VIEWPORT_ROWS)
            {
                sheet->scroll_row = sheet->cursorRow - VIEWPORT_ROWS + 1;
            }
        }
        break;
    case ARROW_LEFT:
        if (sheet->cursorCol > 0)
        {
            sheet->cursorCol--;
            // Scroll viewport if necessary
            if (sheet->cursorCol < sheet->scroll_col)
            {
                sheet->scroll_col = sheet->cursorCol;
            }
        }
        break;

    case ARROW_RIGHT:
        if (sheet->cursorCol < sheet->totalCols - 1)
        {
            sheet->cursorCol++;
            // Scroll viewport if necessary
            if (sheet->cursorCol >= sheet->scroll_col + VIEWPORT_COLS)
            {
                sheet->scroll_col = sheet->cursorCol - VIEWPORT_COLS + 1;
            }
        }
        break;
    case 'n': // Next page (both rows and columns)
        if (sheet->scroll_row + VIEWPORT_ROWS < sheet->totalRows)
        {
            sheet->scroll_row += VIEWPORT_ROWS;
            sheet->cursorRow = sheet->scroll_row;
        }
        if (sheet->scroll_col + VIEWPORT_COLS < sheet->totalCols)
        {
            sheet->scroll_col += VIEWPORT_COLS;
            sheet->cursorCol = sheet->scroll_col;
        }
        break;
    case 'p': // Previous page (both rows and columns)
        if (sheet->scroll_row > 0)
        {
            sheet->scroll_row = (sheet->scroll_row >= VIEWPORT_ROWS) ? sheet->scroll_row - VIEWPORT_ROWS : 0;
            sheet->cursorRow = sheet->scroll_row;
        }
        if (sheet->scroll_col > 0)
        {
            sheet->scroll_col = (sheet->scroll_col >= VIEWPORT_COLS) ? sheet->scroll_col - VIEWPORT_COLS : 0;
            sheet->cursorCol = sheet->scroll_col;
        }
        break;
    case 'r': // Next row page
        if (sheet->scroll_row + VIEWPORT_ROWS < sheet->totalRows)
        {
            sheet->scroll_row += VIEWPORT_ROWS;
            sheet->cursorRow = sheet->scroll_row;
        }
        break;
    case 'c': // Next column page
        if (sheet->scroll_col + VIEWPORT_COLS < sheet->totalCols)
        {
            sheet->scroll_col += VIEWPORT_COLS;
            sheet->cursorCol = sheet->scroll_col;
        }
        break;
    }
}