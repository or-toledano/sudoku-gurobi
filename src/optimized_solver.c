#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "optimized_solver.h"
#include "gurobi_c.h"
#include "board.h"
#include "main_aux.h"

#define NOT_A_VAR (-1)
#define PTR_NUM 8
#define BOARD_IDX(i, j, k, N) ((N) * (N) * (i) + (N) * (j) + (k) - 1)
#define OBJ_CONST 8


/* Checks for optimstatus */
solve_status_t handle_optimstatus(int optimstatus, GRBenv *env, GRBmodel *model, void *pointers[], board_t *board,
                                  int var_num, algo_mode_t mode, double *prob_mat);

/* Updates the board after LP */
solve_status_t
LP_to_board(GRBenv *env, GRBmodel *model, board_t *board, int var_num, void *pointers[], double *prob_mat);

/* Updates the board after ILP */
solve_status_t ILP_to_board(GRBenv *env, GRBmodel *model, board_t *board, int var_num, void *pointers[]);


/*
 * after the use of the function:
 * auto fill cells with only one possibility (won't affect functions like validate() because it calls on a deep copy
 * and:
 * val_num = the sum 0 <= i, j < N of Cij
 * Cij := # possible values of the (i,j) cell in game_board
 * and arr[i][j] will be an array of possible values for the (i,j) cell in game_board
 * arr[i][j][k] != NOT_A_VAR <==> k is a valid value for the cell (i,j)
 * the value of arr[i][j][k] will be the index of the value, in the giant list of all valid values, for all cells
 */
int possible_val_board(board_t *board, int *val_num, int *arr) {
    int i, j, k, vaild_value_num_of_cell, last_valid_value, valid_value_index = 0, N = (board->N);
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (board->board_mat[i][j].val == EMPTY_CELL) {
                vaild_value_num_of_cell = 0;
                for (k = 1; k < N + 1; ++k) { /* counts the possible values */
                    if (is_erroneous_value(board, i, j, k) == SAFE) {
                        last_valid_value = k;
                        ++vaild_value_num_of_cell;
                    }
                }
                if (vaild_value_num_of_cell == 0)
                    return -1;
                if (vaild_value_num_of_cell == 1) {
                    set(board, i, j, last_valid_value);
                } else { /* updates according to the valid values */
                    for (k = 1; k < N + 1; ++k) {
                        if (is_erroneous_value(board, i, j, k) == SAFE) {
                            arr[BOARD_IDX(i, j, k, N)] = valid_value_index;
                            ++valid_value_index;
                        }
                    }
                }
            }
        }
    }
    *val_num = valid_value_index;
    return 0;
}

/* Frees pointers */
void gurobi_free(GRBenv *env, GRBmodel *model, void *pointers[]) {
    int i;
    for (i = 0; i < PTR_NUM; ++i)
        free(pointers[i]);
    GRBfreemodel(model);
    GRBfreeenv(env);
}

/* result[i][k] = # of occurrences of NOT_A_VAR in the ith row of three_dim_mat, with value k*/
void row_notvar_num(int *three_dim_mat, int *result, int N) {
    int i, j, k, occ_num;
    for (k = 1; k < N + 1; ++k) {
        for (i = 0; i < N; ++i) {
            occ_num = 0;
            for (j = 0; j < N; ++j) if (three_dim_mat[BOARD_IDX(i, j, k, N)] == NOT_A_VAR) ++occ_num;
            result[N * i + k - 1] = occ_num;
        }
    }
}

/* result[j][k] = # of occurrences of NOT_A_VAR in the jth col of three_dim_mat, with value k*/
void col_notvar_num(int *three_dim_mat, int *result, int N) {
    int i, j, k, occ_num;
    for (k = 1; k < N + 1; ++k) {
        for (j = 0; j < N; ++j) {
            occ_num = 0;
            for (i = 0; i < N; ++i) if (three_dim_mat[BOARD_IDX(i, j, k, N)] == NOT_A_VAR) ++occ_num;
            result[N * j + k - 1] = occ_num;
        }
    }
}

/* result[l][k] = # of occurrences of NOT_A_VAR in the lth block of three_dim_mat, with value k
 * we count blocks from left to right, top to bottom, N total
 * */
void block_notvar_num(int *three_dim_mat, int *result, int N, int m, int n) {
    int bi, bj, i, j, k, block_idx, occ_num = 0;
    for (k = 1; k < N + 1; ++k) {
        block_idx = 0;
        for (bi = 0; bi < n; ++bi) {
            for (bj = 0; bj < m; ++bj) {
                occ_num = 0;
                for (i = m * bi; i < m * (bi + 1); ++i) {
                    for (j = n * bj; j < n * (bj + 1); ++j)
                        if (three_dim_mat[BOARD_IDX(i, j, k, N)] == NOT_A_VAR) ++occ_num;
                }
                result[N * block_idx + k - 1] = occ_num;
                ++block_idx;
            }
        }
    }
}

/* A good objective function should be random, and additionally, should favour a value k in a cell i,j
 * if, for example, there aren't many possible places for it in the same row/cell/column
 * we count the impossible places for a value k in the same row/col/block of the cell (i,j)
 * and sum them to a number Sijk, and the random part of our function is to pick a random number between
 * 1 and OBJ_CONST*(1+Sijk)
 * so less possible places for value k in the same row/col/block as of (i,j) ==> larger average value for
 * the weight Sijk corresponding to the variable Xijk => Xijk is more important in the maximum optimization
 * problem
 * we multiply by OBJ_CONST and add 1 to Sijk in (OBJ_CONST * (1 + Sijk))
 * so there will be enough random values when Sijk=0
 * */
/* Initializes lb, vtype and objective */
void gurobi_init_vars(board_t *board, int *board_idx_to_var_idx, int var_num, double *obj, double *lb, char *vtype,
                      algo_mode_t mode) {
    int i, j, k, N = board->N, n = board->n, m = board->m, S;
    int *row_imposs_num, *col_imposs_num, *block_imposs_num;
    char var_type;
    if (mode == LP) {
        row_imposs_num = (int *) calloc(N * N, sizeof(int)),
        col_imposs_num = (int *) calloc(N * N, sizeof(int)),
        block_imposs_num = (int *) calloc(N * N, sizeof(int));
        if (row_imposs_num == NULL || col_imposs_num == NULL || block_imposs_num == NULL)
            func_fail_exit("malloc");
        row_notvar_num(board_idx_to_var_idx, row_imposs_num, N);
        col_notvar_num(board_idx_to_var_idx, col_imposs_num, N);
        block_notvar_num(board_idx_to_var_idx, block_imposs_num, N, m, n);
        var_type = GRB_CONTINUOUS;
        for (i = 0; i < N; ++i) {
            for (j = 0; j < N; ++j) {
                for (k = 1; k < N + 1; ++k)
                    if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                        assert(board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] < var_num);
                        /* this is Sijk  */
                        S = row_imposs_num[N * i + k - 1] + col_imposs_num[N * j + k - 1] +
                            block_imposs_num[m * (i / m) + j / n + k - 1];
                        obj[board_idx_to_var_idx[BOARD_IDX(i, j, k, N)]] = 1 + rand() % (OBJ_CONST * (1 + S));
                    }
            }
        }
        free(block_imposs_num);
        free(col_imposs_num);
        free(row_imposs_num);
    } else {
        assert(mode == ILP);
        var_type = GRB_BINARY;
    }
    for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
            for (k = 1; k < N + 1; ++k)
                if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                    lb[board_idx_to_var_idx[BOARD_IDX(i, j, k, N)]] = 0;
                    vtype[board_idx_to_var_idx[BOARD_IDX(i, j, k, N)]] = var_type;
                }
}

/* Adds the "single value in a col" constraint. Returns 0 if successful */
int col_const(int N, int *ind, double *val, GRBenv *env,
              GRBmodel *model, void *pointers[]) {
    int i, j, k, var_count, error;
    int *board_idx_to_var_idx = pointers[1];
    for (k = 1; k < N + 1; ++k) {
        for (j = 0; j < N; ++j) {
            var_count = 0;
            for (i = 0; i < N; ++i) {
                if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                    ind[var_count] = board_idx_to_var_idx[BOARD_IDX(i, j, k, N)];
                    val[var_count] = 1;
                    ++var_count;
                }
            }
            if (var_count > 0) {
                error = GRBaddconstr(model, var_count, ind, val, GRB_EQUAL, 1, "col_const");
                if (error) {
                    printf("ERROR %d GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                    gurobi_free(env, model, pointers);
                    return GRB_FAIL;
                }
            }
        }
    }
    return 0;
}

/* Adds the "single value in a row" constraint. Returns 0 if successful */
int row_const(int N, int *ind, double *val, GRBenv *env,
              GRBmodel *model, void *pointers[]) {
    int i, j, k, var_count, error;
    int *board_idx_to_var_idx = pointers[1];
    for (k = 1; k < N + 1; ++k) {
        for (i = 0; i < N; ++i) {
            var_count = 0;
            for (j = 0; j < N; ++j) {
                if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                    ind[var_count] = board_idx_to_var_idx[BOARD_IDX(i, j, k, N)];
                    val[var_count] = 1;
                    ++var_count;
                }
            }
            if (var_count > 0) {
                error = GRBaddconstr(model, var_count, ind, val, GRB_EQUAL, 1, "row_const");
                if (error) {
                    printf("ERROR %d GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                    gurobi_free(env, model, pointers);
                    return GRB_FAIL;
                }
            }
        }
    }
    return 0;
}

/* Adds the "single value in a block" constraint. Returns 0 if successful */
int block_const(int N, int m, int n, int *ind, double *val, GRBenv *env,
                GRBmodel *model, void *pointers[]) {
    int bi, bj, i, j, k, var_count, error; /* bi, bj are the block coordinates in the block matrix */
    int *board_idx_to_var_idx = pointers[1];
    for (k = 1; k < N + 1; ++k) {
        for (bi = 0; bi < n; ++bi) {
            for (bj = 0; bj < m; ++bj) {
                var_count = 0;
                for (i = m * bi; i < m * (bi + 1); ++i) {
                    for (j = n * bj; j < n * (bj + 1); ++j) {
                        if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                            ind[var_count] = board_idx_to_var_idx[BOARD_IDX(i, j, k, N)];
                            val[var_count] = 1;
                            ++var_count;
                        }
                    }
                }
                if (var_count > 0) {
                    error = GRBaddconstr(model, var_count, ind, val, GRB_EQUAL, 1, "block_const");
                    if (error) {
                        printf("ERROR %d GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                        gurobi_free(env, model, pointers);
                        return GRB_FAIL;
                    }
                }
            }
        }
    }
    return 0;
}

/* Adds the "value of 1-9 for each cell" constraint. Returns 0 if successful */
int each_value_appears_once_const(int N, int *ind, double *val, GRBenv *env,
                                  GRBmodel *model, void *pointers[]) {
    int i, j, k, var_count, error;
    int *board_idx_to_var_idx = pointers[1];
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            var_count = 0;
            for (k = 1; k < N + 1; ++k)
                if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                    ind[var_count] = board_idx_to_var_idx[BOARD_IDX(i, j, k, N)];
                    val[var_count] = 1;
                    ++var_count;
                }
            if (var_count > 0) {
                error = GRBaddconstr(model, var_count, ind, val, GRB_EQUAL, 1, "val_const");
                if (error) {
                    printf("ERROR %d GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                    gurobi_free(env, model, pointers);
                    return GRB_FAIL;
                }
            }
        }
    }
    return 0;
}


typedef enum {
    IND = 0, BOARD_IDX_TO_VAR_IDX, VAL, VTYPE, SOLVED_VARIABLES, LB, OBJ, VAR_NUM_P
} pointer_idx;

solve_status_t optimized_solve(board_t *board, algo_mode_t mode, double *prob_mat) {
    GRBenv *env = NULL;
    GRBmodel *model = NULL;
    int var_num, optimstatus, i, error, N = board->N;
    int *var_num_p = (int *) calloc(1, sizeof(int)), *ind = (int *) calloc(N, sizeof(int)),
            *board_idx_to_var_idx = (int *) calloc(N * N * N, sizeof(int));
    double *val = (double *) calloc(N, sizeof(double)), *obj = NULL, *lb = NULL, *solved_variables = NULL;
    char *vtype = (char *) malloc(N * N * N * sizeof(char));
    void *pointers[PTR_NUM];/* no need for ub=1 in LP because of the lb=0 + sum of vars < 1 (in each cell) constraint */
    if (ind == NULL || board_idx_to_var_idx == NULL || val == NULL || vtype == NULL || var_num_p == NULL)
        func_fail_exit("malloc");
    for (i = 0; i < N * N * N; ++i) board_idx_to_var_idx[i] = NOT_A_VAR; /* start with no vars */
    pointers[IND] = ind, pointers[BOARD_IDX_TO_VAR_IDX] = board_idx_to_var_idx, pointers[VAL] = val,
    pointers[VTYPE] = vtype, pointers[VAR_NUM_P] = var_num_p; /* pointers to free later */
    if (possible_val_board(board, var_num_p, board_idx_to_var_idx)) { /* figure out the relevant vars */
        gurobi_free(env, model, pointers);
        return SOLVE_FAIL; /* an impossible cell after filling some cells with only one possible value */
    }
    var_num = *var_num_p;
    solved_variables = (double *) calloc(var_num, sizeof(double)); /* all of the variables Xijk */
    lb = (double *) calloc(var_num, sizeof(double));    /* lower bound */
    if (solved_variables == NULL || lb == NULL) func_fail_exit("malloc");
    pointers[SOLVED_VARIABLES] = solved_variables, pointers[LB] = lb;
    if (mode == LP) {
        obj = calloc(var_num, sizeof(double));
        if (obj == NULL) func_fail_exit("calloc");
    }
    pointers[OBJ] = obj;
    gurobi_init_vars(board, board_idx_to_var_idx, var_num, obj, lb, vtype, mode); /* initialize lb, vtype */
    error = GRBloadenv(&env, "sudoku.log");
    if (error) {
        printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) {
        printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    error = GRBnewmodel(env, &model, "sudoku", var_num, obj, lb, NULL, vtype, NULL);
    if (error) {
        printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    if (mode == LP) {
        error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
        if (error) {
            printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));
            gurobi_free(env, model, pointers);
            return GRB_FAIL;
        }
    }
    if (block_const(N, board->m, board->n, ind, val, env, model, pointers) ||
        col_const(N, ind, val, env, model, pointers) ||
        row_const(N, ind, val, env, model, pointers) ||
        each_value_appears_once_const(N, ind, val, env, model, pointers))
        return GRB_FAIL; /* check if adding the constraints had failed */
    error = GRBoptimize(model);
    if (error) {
        printf("ERROR %d GRBoptimize(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    error = GRBwrite(model, "sudoku.lp");
    if (error) {
        printf("ERROR %d GRBwrite(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) {
        printf("ERROR %d GRBgetintattr(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    return handle_optimstatus(optimstatus, env, model, pointers, board, var_num, mode, prob_mat);
}

solve_status_t handle_optimstatus(int optimstatus, GRBenv *env, GRBmodel *model, void *pointers[],
                                  board_t *board, int var_num, algo_mode_t mode, double *prob_mat) {
    if (optimstatus == GRB_OPTIMAL) {
        if (mode == LP)
            return LP_to_board(env, model, board, var_num, pointers, prob_mat);
        else
            return ILP_to_board(env, model, board, var_num, pointers);
    } else if (optimstatus == GRB_INF_OR_UNBD || optimstatus == GRB_UNBOUNDED || optimstatus == GRB_INFEASIBLE) {
        gurobi_free(env, model, pointers);
        return SOLVE_FAIL;
    } else {
        printf("ERROR: Gurobi optimization had failed / was stopped early: %d\n", optimstatus);
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
}


solve_status_t LP_to_board(GRBenv *env, GRBmodel *model, board_t *board, int var_num, void *pointers[],
                           double *prob_mat) {
    int i, j, k, error, N = board->N;
    int *board_idx_to_var_idx = pointers[BOARD_IDX_TO_VAR_IDX];
    double *variables = pointers[SOLVED_VARIABLES];
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, var_num, variables);
    if (error) {
        printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            for (k = 1; k < N + 1; ++k) {
                if (board->board_mat[i][j].type == EMPTY) {
                    if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] == NOT_A_VAR)
                        prob_mat[BOARD_IDX(i, j, k, N)] = 0;
                    else
                        prob_mat[BOARD_IDX(i, j, k, N)] = variables[board_idx_to_var_idx[BOARD_IDX(i, j, k, N)]];

                } else {
                    if (k == board->board_mat[i][j].val)
                        prob_mat[BOARD_IDX(i, j, k, N)] = 1;
                    else
                        prob_mat[BOARD_IDX(i, j, k, N)] = 0;
                }
            }
        }
    }
    gurobi_free(env, model, pointers);
    return SOLVE_SUCCESS;
}


solve_status_t ILP_to_board(GRBenv *env, GRBmodel *model, board_t *board, int var_num, void *pointers[]) {
    int i, j, k, error, N = board->N;
    int *board_idx_to_var_idx = pointers[BOARD_IDX_TO_VAR_IDX];
    double *variables = pointers[SOLVED_VARIABLES];
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, var_num, variables);
    if (error) {
        printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));
        gurobi_free(env, model, pointers);
        return GRB_FAIL;
    }
    for (i = 0; i < board->N; ++i)
        for (j = 0; j < board->N; ++j)
            for (k = 1; k < board->N + 1; ++k)
                if (board_idx_to_var_idx[BOARD_IDX(i, j, k, N)] != NOT_A_VAR) {
                    if (variables[board_idx_to_var_idx[BOARD_IDX(i, j, k, N)]] == 1)
                        set(board, i, j, k);
                }
    gurobi_free(env, model, pointers);
    return SOLVE_SUCCESS;
}
