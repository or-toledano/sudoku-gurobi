/* list module - a doubly linked list, with actions/moves as nodes */
#ifndef SUDOKU_PROJECT_LIST_H
#define SUDOKU_PROJECT_LIST_H

#include "board.h"

#define LIST_START -1 /*marks the start of the list*/

/* Struct double_node_t, represents an action/move on the board
 * num_of_act - number of change cells in the action
 * act_list - a list of the change done save in the according to the following format:
 * of every 0<=k<num_of_act act_list[3*k],act_list[3*k+1] is a changed cell (in i,l indexing)
 * and act_list[3*k+2] is the value needed to revert the change.*/
typedef struct double_node_s {
    int num_of_act;
    int *act_list;
    struct double_node_s *prev;
    struct double_node_s *next;
} double_node_t;

/* A list of moves, current represents the current state so enacting current will
 * undo the last operation (enacting current->next will redo, see enact)*/
typedef struct {
    double_node_t *current;
} list_t;

/*to_print_output_t enum - passed as argument to a function
 * to indicate if it should print it's output*/
typedef enum {
    DONT_PRINT, PRINT_OUTPUT
} to_print_output_t;

/*Enact the changes of action on board assuming the it is valid
 * and set action act_list the act_list need to revert the change (so
 * act_list needed to undo become the act_list that needed or redo and vice versa)
 * the function also may print the change according to the print parameter.*/
void enact(board_t *board, double_node_t *action, to_print_output_t print);

/*Construct a list with one node
 * that represent the beginning of the game
 * (num_of_act=LIST_START,act_list=NULL)*/
list_t *list_ctor();

/*Destructor for the list*/
void list_dtor(list_t *list);

/* Adds a new move to the list */
void list_add(list_t *list, int num_of_act, int *act_list);

/*Clears the redo part of the list*/
void list_clear_redo(list_t *list);


#endif
