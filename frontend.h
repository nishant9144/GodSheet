#ifndef FRONTEND_H
#define FRONTEND_H

#include "spreadsheet.h"
#define VIEWPORT_ROWS 10
#define VIEWPORT_COLS 10
#define CELL_WIDTH 12
#define MAX_CELL_LENGTH 50
#define CELL_PADDING 1
#define MAX_FORMULA_LEN 100
#define MAX_DEPS 100
#define SLEEP_TIME_MULTIPLIER 1000000
#define MAX_ROWS 999
#define MAX_COLS 18278
#define VIEW_MODE 0
#define EDIT_MODE 1
#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_RIGHT 'C'
#define ARROW_LEFT 'D'
#define ESC '\x1b'

// Terminal control sequences
#define CLEAR_SCREEN "\x1b[2J\x1b[H"

/* Terminal configuration functions */
void configure_terminal(void);
void restore_terminal(void);
void clear_screen(void);

/* Display functions */
void clear_screen(void);
void display_status_bar(Spreadsheet *sheet);
void display_viewport(Spreadsheet *sheet);

/* Scroll handler */
// void handle_scroll(Spreadsheet *sheet, char direction);
char readArrowKeys();
void handleNavigation(Spreadsheet *sheet, char key);

#endif
