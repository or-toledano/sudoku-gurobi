/* optimized_solver module - solve the board using LP/ILP */
#ifndef SUDOKU_PROJECT_OPTIMIZED_SOLVER_H
#define SUDOKU_PROJECT_OPTIMIZED_SOLVER_H

#include "board.h"

/* status for optimized_solve:
 * SOLVE_SUCCESS - board is solvable
 * SOLVE_FAIL - board is unsolvable
 * GRB_FAIL - gurobi failed during solution. A proper error message will display before the return from optimized_solve
 * */
typedef enum {
    SOLVE_SUCCESS, SOLVE_FAIL, GRB_FAIL
} solve_status_t;

/* LP/ILP flag */
typedef enum {
    LP, ILP
} algo_mode_t;

/* solve the board using LP/ILP flag which is saved in mode
 * prob_mat is for LP, saves probs of variables Xijk, between 0 and 1
 * */
solve_status_t optimized_solve(board_t *board, algo_mode_t mode, double *prob_mat);

#endif

