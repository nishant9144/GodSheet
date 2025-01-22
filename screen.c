#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define SHEET_WIDTH 10
#define SHEET_HEIGHT 10
#define CELL_WIDTH 12  // Configurable cell width
#define CELL_PADDING 1 // Space between cells
#define MAX_ROWS 999
#define MAX_COLS 18278 // ZZZ999 = 26^3 + 26^2 + 26
#define MAX_CELL_LENGTH 50
#define VIEW_MODE 0
#define EDIT_MODE 1
#define ESC '\x1b'
#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_RIGHT 'C'
#define ARROW_LEFT 'D'

// Structure to store cell data
typedef struct
{
    int content;
    int isFormula;
    char formula[MAX_CELL_LENGTH]; // Store the original formula
} Cell;

// Structure to store spreadsheet data
typedef struct
{
    Cell **cells;
    int totalRows;
    int totalCols;
    int currentTopRow;
    int currentLeftCol;
    int mode;
    int cursorRow;
    int cursorCol;
    int viewportRows;
    int viewportCols;
} Spreadsheet;

// Function to convert column reference (e.g., "AAA") to number (0-based)
int colRefToNum(const char *colRef)
{
    int result = 0;
    int len = strlen(colRef);
    for (int i = 0; i < len; i++)
    {
        if (!isupper(colRef[i]))
            return -1;
        result = result * 26 + (colRef[i] - 'A');
    }
    return result;
}

// Function to convert column number to reference (e.g., 0 -> "A", 25 -> "Z", 26 -> "AA")
void numToColRef(int num, char *colRef)
{
    int idx = 0;
    if (num < 0)
    {
        colRef[0] = '\0';
        return;
    }

    while (num >= 0)
    {
        colRef[idx++] = 'A' + (num % 26);
        num = num / 26 - 1;
        if (num < 0)
            break;
    }
    colRef[idx] = '\0';

    // Reverse the string
    for (int i = 0; i < idx / 2; i++)
    {
        char temp = colRef[i];
        colRef[i] = colRef[idx - 1 - i];
        colRef[idx - 1 - i] = temp;
    }
}

// Function to parse cell reference (e.g., "A5" -> row 4, col 0)
int parseCellRef(const char *ref, int *row, int *col)
{
    char colRef[4] = {0};
    int i = 0;

    // Extract column reference
    while (ref[i] && isupper(ref[i]) && i < 3)
    {
        colRef[i] = ref[i];
        i++;
    }
    colRef[i] = '\0';

    // Extract row number
    int rowNum = atoi(ref + i);
    if (rowNum <= 0 || rowNum > MAX_ROWS)
        return 0;

    *col = colRefToNum(colRef);
    *row = rowNum - 1;

    return (*col >= 0 && *col < MAX_COLS);
}

int evaluateFormula(Spreadsheet *sheet, int row, int col, char *formula, int *error)
{
    *error = 0;
    char *token = strtok(formula, "+="); // Splits a string into pieces using delimiters '+' or '='
    int result = 0;
    int first = 1;

    while (token != NULL)
    {
        while (*token == ' ')
            token++; // Skip spaces
        if (strlen(token) == 0)
        {
            token = strtok(NULL, "+=");
            continue;
        }

        int cellRow, cellCol;
        if (parseCellRef(token, &cellRow, &cellCol))
        {
            // Check if cell reference is valid
            if (cellRow >= sheet->totalRows || cellCol >= sheet->totalCols)
            {
                *error = 1;
                return 0;
            }

            // Check for circular reference
            if (cellRow == row && cellCol == col)
            {
                *error = 1;
                return 0;
            }

            if (first)
            {
                result = sheet->cells[cellRow][cellCol].content;
                first = 0;
            }
            else
            {
                result += sheet->cells[cellRow][cellCol].content;
            }
        }
        else
        {
            // Try to parse as number
            char *endptr;
            int num = strtol(token, &endptr, 10);
            if (*endptr != '\0' && *endptr != ' ')
            {
                *error = 1;
                return 0;
            }
            if (first)
            {
                result = num;
                first = 0;
            }
            else
            {
                result += num;
            }
        }
        token = strtok(NULL, "+=");
    }
    return result;
}

// Function to process cell command (e.g., "A5=3" or "A5=B1+B2")
void processCellCommand(Spreadsheet *sheet, char *command)
{
    char *equals = strchr(command, '=');
    if (!equals)
        return;

    *equals = '\0';
    char *cellRef = command;
    char *value = equals + 1;

    // Remove spaces
    while (*cellRef == ' ')
        cellRef++;
    while (*value == ' ')
        value++;

    int row, col;
    if (!parseCellRef(cellRef, &row, &col) || row >= sheet->totalRows || col >= sheet->totalCols)
    {
        printf("Invalid cell reference\n");
        return;
    }

    if (value[0] == '=')
    {
        // Formula
        int content_of_cell = sheet->cells[row][col].content;
        sheet->cells[row][col].isFormula = 1;
        strncpy(sheet->cells[row][col].formula, value + 1, MAX_CELL_LENGTH - 1);
        int error;
        sheet->cells[row][col].content = evaluateFormula(sheet, row, col, value + 1, &error);
        if (error)
        {
            sheet->cells[row][col].isFormula = 0;
            printf("Formula error\n");
            sheet->cells[row][col].content = content_of_cell;
        }
    }
    else
    {
        // Direct value
        sheet->cells[row][col].isFormula = 0;
        sheet->cells[row][col].content = atoi(value);
    }
}

// Function to configure terminal for raw input
void configureTerminal()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to restore terminal settings
void restoreTerminal()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to clear the terminal screen
void clearScreen()
{
    printf("\x1b[2J\x1b[H");
}

// Function to set text color
void setColor(int highlight)
{
    if (highlight)
    {
        printf("\x1b[47m\x1b[30m"); // Black text on white background
    }
    else
    {
        printf("\x1b[0m"); // Reset color
    }
}

// Function to initialize the spreadsheet
Spreadsheet *initSpreadsheet(int rows, int cols)
{
    Spreadsheet *sheet = (Spreadsheet *)malloc(sizeof(Spreadsheet)); // Created a pointer of the sheet
    sheet->totalRows = rows;
    sheet->totalCols = cols;
    sheet->currentTopRow = 0;
    sheet->currentLeftCol = 0;
    sheet->mode = VIEW_MODE;
    sheet->cursorRow = 0;
    sheet->cursorCol = 0;
    sheet->viewportRows = SHEET_HEIGHT;
    sheet->viewportCols = SHEET_WIDTH;

    // Allocate memory for cells
    sheet->cells = (Cell **)malloc(rows * sizeof(Cell *));
    for (int i = 0; i < rows; i++)
    {
        sheet->cells[i] = (Cell *)malloc(cols * sizeof(Cell));
        for (int j = 0; j < cols; j++)
        {
            // Initialize all cells with "0"
            sheet->cells[i][j].content = 0;
            sheet->cells[i][j].isFormula = 0;
        }
    }
    return sheet;
}

// Function to display column headers (A, B, C, ...)
void displayColumnHeaders(int startCol, int endCol)
{
    printf("      "); // Space for row numbers
    for (int j = startCol; j < endCol; j++)
    {
        char colRef[4];
        numToColRef(j, colRef);
        printf("%-*s", CELL_WIDTH, colRef);
    }
    printf("\n");
}

// Function to display the spreadsheet
void displaySheet(Spreadsheet *sheet)
{
    clearScreen();
    int endRow = sheet->currentTopRow + sheet->viewportRows;
    int endCol = sheet->currentLeftCol + sheet->viewportCols;

    if (endRow > sheet->totalRows)
    {
        endRow = sheet->totalRows;
    }
    if (endCol > sheet->totalCols)
    {
        endCol = sheet->totalCols;
    }

    // Get current cell reference
    char currentColRef[4];
    numToColRef(sheet->cursorCol, currentColRef);

    // Display mode and navigation info
    printf("Mode: %s    Page: Row %d-%d, Col %c-%c\n",
           sheet->mode == VIEW_MODE ? "VIEW" : "EDIT",
           sheet->currentTopRow + 1, endRow,
           'A' + sheet->currentLeftCol, 'A' + endCol - 1);

    // Display column headers
    displayColumnHeaders(sheet->currentLeftCol, endCol);

    // Print horizontal separator
    printf("    +-");
    for (int j = sheet->currentLeftCol; j < endCol; j++)
    {
        for (int k = 0; k < CELL_WIDTH; k++)
            printf("-");
        printf("-");
    }
    printf("\n");

    // Display cells
    for (int i = sheet->currentTopRow; i < endRow; i++)
    {
        printf("%4d |", i + 1); // Row numbers with border
        for (int j = sheet->currentLeftCol; j < endCol; j++)
        {
            printf(" "); // Cell padding
            if (i == sheet->cursorRow && j == sheet->cursorCol)
            {
                setColor(1); // Highlight
                if (sheet->cells[i][j].isFormula)
                    printf("=%-*s", CELL_WIDTH - 1, sheet->cells[i][j].formula);
                else
                    printf("%-*d", CELL_WIDTH - 1, sheet->cells[i][j].content);
                setColor(0); // Reset
            }
            else
            {
                if (sheet->cells[i][j].isFormula)
                    printf("=%-*s", CELL_WIDTH - 1, sheet->cells[i][j].formula);
                else
                    printf("%-*d", CELL_WIDTH - 1, sheet->cells[i][j].content);
            }
        }
        printf("\n");
    }

    // Print bottom border
    printf("    +-");
    for (int j = sheet->currentLeftCol; j < endCol; j++)
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
}

// Modified editCell function to handle cell commands
void editCell(Spreadsheet *sheet)
{
    printf("Enter command (e.g., A5=3 or A5=B1+B2): ");
    restoreTerminal();

    char input[MAX_CELL_LENGTH];
    fgets(input, MAX_CELL_LENGTH, stdin);
    input[strcspn(input, "\n")] = 0;
    // the code uses the strcspn function to find the
    // position of the newline character (\n) within the input string.

    if (strlen(input) > 0)
    {
        processCellCommand(sheet, input);
    }

    configureTerminal();
}

// Function to handle navigation
void handleNavigation(Spreadsheet *sheet, char key)
{
    switch (key)
    {
    case ARROW_UP:
        if (sheet->cursorRow > 0)
        {
            sheet->cursorRow--;
            // Scroll viewport if necessary
            if (sheet->cursorRow < sheet->currentTopRow)
            {
                sheet->currentTopRow = sheet->cursorRow;
            }
        }
        break;
    case ARROW_DOWN:
        if (sheet->cursorRow < sheet->totalRows - 1)
        {
            sheet->cursorRow++;
            // Scroll viewport if necessary
            if (sheet->cursorRow >= sheet->currentTopRow + sheet->viewportRows)
            {
                sheet->currentTopRow = sheet->cursorRow - sheet->viewportRows + 1;
            }
        }
        break;
    case ARROW_LEFT:
        if (sheet->cursorCol > 0)
        {
            sheet->cursorCol--;
            // Scroll viewport if necessary
            if (sheet->cursorCol < sheet->currentLeftCol)
            {
                sheet->currentLeftCol = sheet->cursorCol;
            }
        }
        break;
    case ARROW_RIGHT:
        if (sheet->cursorCol < sheet->totalCols - 1)
        {
            sheet->cursorCol++;
            // Scroll viewport if necessary
            if (sheet->cursorCol >= sheet->currentLeftCol + sheet->viewportCols)
            {
                sheet->currentLeftCol = sheet->cursorCol - sheet->viewportCols + 1;
            }
        }
        break;
    case 'n': // Next page (both rows and columns)
        if (sheet->currentTopRow + sheet->viewportRows < sheet->totalRows)
        {
            sheet->currentTopRow += sheet->viewportRows;
            sheet->cursorRow = sheet->currentTopRow;
        }
        if (sheet->currentLeftCol + sheet->viewportCols < sheet->totalCols)
        {
            sheet->currentLeftCol += sheet->viewportCols;
            sheet->cursorCol = sheet->currentLeftCol;
        }
        break;
    case 'p': // Previous page (both rows and columns)
        if (sheet->currentTopRow > 0)
        {
            sheet->currentTopRow = (sheet->currentTopRow >= sheet->viewportRows) ? sheet->currentTopRow - sheet->viewportRows : 0;
            sheet->cursorRow = sheet->currentTopRow;
        }
        if (sheet->currentLeftCol > 0)
        {
            sheet->currentLeftCol = (sheet->currentLeftCol >= sheet->viewportCols) ? sheet->currentLeftCol - sheet->viewportCols : 0;
            sheet->cursorCol = sheet->currentLeftCol;
        }
        break;
    case 'r': // Next row page
        if (sheet->currentTopRow + sheet->viewportRows < sheet->totalRows)
        {
            sheet->currentTopRow += sheet->viewportRows;
            sheet->cursorRow = sheet->currentTopRow;
        }
        break;
    case 'c': // Next column page
        if (sheet->currentLeftCol + sheet->viewportCols < sheet->totalCols)
        {
            sheet->currentLeftCol += sheet->viewportCols;
            sheet->cursorCol = sheet->currentLeftCol;
        }
        break;
    }
}
// Function to read arrow keys
char readArrowKeys()
{
    char c = getchar();
    if (c == ESC)
    {
        c = getchar(); // Skip '['
        if (c == '[')
        {
            return getchar(); // Return the actual arrow key code
        }
        return c;
    }
    return c;
}

int main(int argc, char *argv[])
{
    int rows, cols;

    if (argc != 3)
    {
        printf("Usage: %s rows cols\n", argv[0]);
        return 1;
    }

    rows = atoi(argv[1]);
    cols = atoi(argv[2]);

    if (rows <= 0 || rows > MAX_ROWS || cols <= 0 || cols > MAX_COLS)
    {
        printf("Invalid dimensions. Rows: 1-%d, Cols: 1-%d\n", MAX_ROWS, MAX_COLS);
        return 1;
    }

    Spreadsheet *sheet = initSpreadsheet(rows, cols);
    configureTerminal();

    char input;
    while (1)
    {
        system("clear"); // Clear the terminal screen
        displaySheet(sheet);
        input = readArrowKeys();

        if (input == 'q' || input == 'Q')
        {
            break;
        }
        else if (input == 'm' || input == 'M')
        {
            sheet->mode = (sheet->mode == VIEW_MODE) ? EDIT_MODE : VIEW_MODE;
        }
        else if (input == '\n' && sheet->mode == EDIT_MODE)
        {
            editCell(sheet);
        }
        else
        {
            handleNavigation(sheet, input);
        }
    }

    restoreTerminal();

    for (int i = 0; i < sheet->totalRows; i++)
    {
        free(sheet->cells[i]);
    }
    free(sheet->cells);
    free(sheet);

    return 0;
}