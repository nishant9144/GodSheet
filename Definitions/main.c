#include "../Declarations/frontend.h"
#include "../Declarations/backend.h"
#include "../Declarations/parser.h"
#include "../Declarations/ds.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s rows cols\n", argv[0]);
        return 1;
    }

    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);

    // Debug print to confirm argument values

    if (rows < 1 || cols < 1 || rows > MAX_ROWS || cols > MAX_COLS)
    {
        fprintf(stderr, "Invalid dimensions. MAX_ROWS: %d, MAX_COLS: %d\n", MAX_ROWS, MAX_COLS);
        return 1;
    }

    Spreadsheet *sheet = create_spreadsheet(rows, cols);
    gettimeofday(&sheet->last_cmd_time, NULL);  // Initialize last_cmd_time properly

    configure_terminal();

    run_ui(sheet);

    restore_terminal();
    free_spreadsheet(sheet);
    return 0;
}