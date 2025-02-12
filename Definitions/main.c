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
    if (rows < 1 || cols < 1 || rows > MAX_ROWS || cols > MAX_COLS)
    {
        fprintf(stderr, "Invalid dimensions\n");
        return 1;
    }

    Spreadsheet *sheet = create_spreadsheet(rows, cols);
    configure_terminal();

    char input;
    while (1)
    {
        system("clear");
        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        // Here checker should be created to display the screen or not
        display_viewport(sheet);

        input = readArrowKeys();

        if (input == 'q' || input == 'Q')
        {
            printf("\n");
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

        struct timeval end_time;
        gettimeofday(&end_time, NULL);
        sheet->last_cmd_time = end_time;

    }

    restore_terminal();
    free_spreadsheet(sheet);
    return 0;
}