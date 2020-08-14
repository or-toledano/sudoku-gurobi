/* game module - defines the game structure and its functions*/
#ifndef SUDOKU_PROJECT_GAME_H
#define SUDOKU_PROJECT_GAME_H


#include "board.h"
#include "list.h"
#include "parser.h"


#define MAX_GEN_ITER 1000

/*game_mode_t - represents the game modes*/
typedef enum {
    M_INIT = F_INIT, M_EDIT = F_EDIT, M_SOLVE = F_SOLVE
} game_mode_t;

/*struct game
 * The representation of the game
 * there is one occurrence of the struct crated in main
 * and used throughout all of the life of the program
 * and pass as an argument to game's function.*/
typedef struct game_s {
    board_t *board;/*the board of the game*/
    list_t *list;/*the undo\redo list*/
    game_mode_t mode;/*the mode of the game*/
    int mark_errors;/*1 for marking error 0 not marking*/
} game_t;

/*Game_ctor - constructor of game starting at init with mark_errors=1
 * and NULL board and list*/
game_t *game_ctor();

/*Solve - implementation the of solve command, load board saved in path X if board cannot be loaded
 * a proper error message is printed otherwise the board is saved into board in game and game mode is set to
 * M_SOLVE.*/
void solve(game_t *game, char *X);

/*Edit - implementation the of edit command, if X is not NULL the function loads board saved in path X,
 * if board cannot be loaded a proper error message is printed. if X=NULL an empty board with n,m=3.
 * if the board is successfully crated (in the way that X determines) it's saved into board in game
 * and game mode is set to M_EDIT*/
void edit(game_t *game, char *X);

/*Save - implementation of the save command, check if the mode of the game allow the command
 * if it does not allow a proper error message will be printed otherwise the command cannot be saved
 * for any reason (the board cannot be saved in current mode or a error occurred) the user is notified
 * by proper error message*/
void save(game_t *game, char *X);

/*Mark_errors - implementation of the mark_errors command, change mark_errors in the
 * game parameter if an error occurs (mark_errors not in the right range or mode does not support command)
 *the user is notified by proper error message*/
void mark_errors(game_t *game, int mark_errors);

/*User_print_board - implementation of the print_board command,if the mode of game
 * does not support te command the user is notified by proper error message otherwise
 * the board is printed*/
void user_print_board(game_t *game);

/*User_set - implementation of the set command, sets the (x,y) cell of the
 * board of game to z if an error occurs (value out of range, mode does not support command, the cell is fixed)
 * the user is notified by proper error message.
 * if the command is successful the undo/redo list is updated and the board is check
 * for game end condition and the user informed if needed (in solve mode, when board is full)*/
void user_set(game_t *game, int x, int y, int z);

/*Validate - implementation of the validate command,
 * uses ILP to validate the board and print the result to the user
 * if an error occurs the user is notified by proper error message*/
void validate(game_t *game);

/*Hint - implementation of the hint command,
 *uses ILP to solve the board of game and prints a hint to user
 * that contains the value of cell (x,y) to the user
 * if an error occurs the user is notified by proper error message*/
void hint(game_t *game, int x, int y);

/*Generate - implementation of the generate command,
 * fill X random empty cell solves game's board with ILP
 * and empty all but y cells.
 * if an error occurs the user is notified by proper error message.
 * if the command is successful the undo/redo list is updated.*/
void generate(game_t *game, int x, int y);

/*Guess - implementation of the guess command,
 * uses LP to fill empty cell in game's board that has
 * score a legal value with score above X (or equal to it)
 * if there are multiple of this values exist the value filled
 * at random according to the scores of the possible values.
 * if an error occurs the user is notified by proper error message.
 * if the command is successful the undo/redo list is updated and the board is check
 * for game end condition and the user informed if needed.*/
void guess(game_t *game, double X);

/*Guess_hint - implementation of the guess_hint command,
 * uses LP to give scores to the legal value of cell (x,y)
 * in game's board and print the score of the non zero scored
 * legal value to the user.
 * if an error occurs the user is notified by proper error message.*/
void guess_hint(game_t *game, int x, int y);

/*Num_sol - implementation of the num_solutions command,
 * uses exhaustive backtracking to find the number of solution
 * for the board and print it to the user.
 * if an error occurs the user is notified by proper error message.*/
void num_sol(game_t *game);

/*Autofill - implementation of the Autofill command,
 * fills all cells in game's board that have one legal value.
 * if an error occurs the user is notified by proper error message.
 * if the command is successful the undo/redo list is updated and the board is check
 * for game end condition and the user informed if needed.*/
void autofill(game_t *game);

/*Undo - implementation of the undo command,
 * undo the last action that change the board (or a set that does not
 * necessarily change the board) and print the changes to the user.
 * if an error occurs the user is notified by proper error message.*/
void undo(game_t *game);

/*Redo - implementation of the redo command,
 * redo the last action that change the board (or a set that does not
 * necessarily change the board) and print the changes to the user.
 * if an error occurs the user is notified by proper error message.*/
void redo(game_t *game);

/*Reset - implementation of the reset command,
 * undo all action in game's undo/redo list.
 * if an error occurs the user is notified by proper error message.*/
void reset(game_t *game);

/*Game_exit - implementation of the exit command,
 * cleanly exits the game.*/
void game_exit(game_t *game);

#endif


