#ifndef FRONTEND_H
#define FRONTEND_H

#include "spreadsheet.h"

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
