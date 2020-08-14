/*solution_counter module - iterative implementation of exhaustive backtracking using stack*/
#ifndef SUDOKU_PROJECT_SOULTION_CONTER_H
#define SUDOKU_PROJECT_SOULTION_CONTER_H

/*Struct call_t, the struct represent a recursive call
 * c is the index of the cell in the empty cell list.
 * z is the value the that fills th cell*/
typedef struct call_s {
    int z;
    int c;
} call_t;

/*Counts the number of solution of the board via exhaustive backtracking
 * using stack of call_t.*/
int count_sol(board_t *board);

#endif
