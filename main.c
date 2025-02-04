#include "spreadsheet.h"
#include "frontend.h"
#include "backend.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

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

        // (Status display is handled in display_viewport via display_status_bar.)
    }

    restore_terminal();
    destroy_spreadsheet(sheet);
    return 0;
}