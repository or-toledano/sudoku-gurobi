/* main_aux module - functions used by main */
#ifndef SUDOKU_PROJECT_MAIN_AUX_H
#define SUDOKU_PROJECT_MAIN_AUX_H

#include "game.h"

/*Return status for functions.*/
#define SUCCESS 0
#define FAILURE 1


/* Func_fail_exit - Exits with an error related to a function
 * in the scope of the project it is used for
 * for exit on unrecoverable error of libray
 * function (memory errors or error while reading
 * from stdin)*/
void func_fail_exit(const char *msg);


/* Parse_and_dispatch_commands - reads commands
 * from stdin and dispatch them or prints proper
 * message if there is error in the command, until
 * an exit/EOF received or unrecoverable error occurs.*/
void parse_and_dispatch_commands(game_t *game);


#endif
