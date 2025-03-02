#ifndef FRONTEND_H
#define FRONTEND_H

#include "header.h"
#include "ds.h"

#define VIEWPORT_ROWS 10
#define VIEWPORT_COLS 10

#define CELL_WIDTH 8
#define MAX_CELL_LENGTH 100

// Terminal control sequences
#define CLEAR_SCREEN "\033[H\033[J"

void display_viewport(Spreadsheet *sheet);

void scroll_to(Spreadsheet *sheet, int row, int col);

void handle_scroll(Spreadsheet *sheet, char direction);
void run_ui(Spreadsheet *sheet);

#endif
