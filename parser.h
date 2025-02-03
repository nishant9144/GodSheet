#ifndef PARSER_H
#define PARSER_H

#include "spreadsheet.h"

/**
 * process_command:
 *   Parses a full command (e.g., scroll commands or a cell assignment such as A1=2+3)
 *   and dispatches the work to the appropriate functions.
 *
 * @param sheet: Pointer to the Spreadsheet
 * @param input: The full input string (without a trailing newline)
 */
void process_command(Spreadsheet *sheet, char *input);

#endif
