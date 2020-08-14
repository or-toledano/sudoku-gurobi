/* files module - file I/O handling */
#ifndef SUDOKU_PROJECT_FILES_H
#define SUDOKU_PROJECT_FILES_H

#include "board.h"

/*Save board to file treat USER cell type as user_cell
 * if an if an error occurred proper error message will be printed*/
void save_to_file(board_t *board, const char *file_name, entry_type_t user_cell);

/*Read a board from file_name treat FIXED cell type as fixed_cell
 * if an if an error occurred proper error message will be printed*/
board_t *read_from_file(const char *file_name, entry_type_t fixed_cell);

#endif

