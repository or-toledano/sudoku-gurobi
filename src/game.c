#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "game.h"
#include "board.h"
#include "main_aux.h"
#include "files.h"
#include "solution_counter.h"
#include "optimized_solver.h"
#include "list.h"

game_t *game_ctor() {
    game_t *game = malloc(sizeof(game_t));
    game->board = NULL;
    game->mode = M_INIT;
    game->list = NULL;
    game->mark_errors = 1;
    return game;
}

/*Destructor or game_t*/
static void game_dtor(game_t *game) {
    if (game != NULL) {
        list_dtor(game->list);
        board_dtor(game->board);
        free(game);
    }
}

/*If the game at solve mode and the board if full
 * the function notifies the user if the board is erroneous
 * and if it's does not the function cleanly change to mode
 * to init mode.
 * In any way the function prints the board*/
static void check_game_end(game_t *game) {
    board_t *board = game->board;
    int f = board->filled_num;
    int N = board->N;
    if (game->mode == M_SOLVE && f == N * N) {
        if (board->error_num > 0) {
            printf("Board solution is erroneous\n");
        } else {
            printf("Congratulation you have solved the board!\n");
            list_dtor(game->list);
            game->list = NULL;
            board_dtor(game->board);
            game->board = NULL;
            game->mode = M_INIT;
        }
    }
}

void solve(game_t *game, char *X) {
    board_t *board;
    board = read_from_file(X, FIXED);
    if (board != NULL) {
        board_dtor(game->board);
        game->board = board;
        game->mode = M_SOLVE;
        list_dtor(game->list);
        game->list = list_ctor();
        user_print_board(game);
        check_game_end(game);
    }
}

void edit(game_t *game, char *X) {
    board_t *board;
    if (X == NULL)
        board = board_ctor(3, 3);
    else
        board = read_from_file(X, USER);
    if (board != NULL) {
        board_dtor(game->board);
        game->board = board;
        game->mode = M_EDIT;
        list_dtor(game->list);
        game->list = list_ctor();
        user_print_board(game);
    }
}

void save(game_t *game, char *X) {
    solve_status_t status;
    board_t *copy;
    if (game->mode == M_SOLVE)
        save_to_file(game->board, X, USER);
    else {
        if (game->board->error_num > 0)
            printf("Error: cannot save an erroneous in edit mode\n");
        else {
            copy = copy_board(game->board);
            status = optimized_solve(copy, ILP, NULL);
            switch (status) {
                case SOLVE_SUCCESS:
                    save_to_file(game->board, X, FIXED);
                    break;
                case SOLVE_FAIL:
                    printf("Error: cannot save a board unsolvable in edit mode\n");
                    break;
                case GRB_FAIL:
                    printf("Error: validation failed do to guroi error, can't save the board\n");
                    break;
            }
            board_dtor(copy);
        }
    }
}

void mark_errors(game_t *game, int mark_errors) {
    if (mark_errors < 0 || mark_errors > 1)
        printf("Error: mark_errors command parameter should be 0 or 1\n");
    else
        game->mark_errors = mark_errors;
}

void user_print_board(game_t *game) {
    print_board(game->board, game->mark_errors || (game->mode == M_EDIT));
}

void user_set(game_t *game, int x, int y, int z) {
    int N = game->board->N;
    int *act_list;
    int old_val;
    if (x < 1)
        printf("Error: parameter 1 (x) should be positive\n");
    else if (x > N)
        printf("Error: parameter 1 (x) should be smaller or equal to N=%d\n", N);
    else if (y < 1)
        printf("Error: parameter 2 (y) should be positive\n");
    else if (y > N)
        printf("Error: parameter 2 (y) should be smaller or equal to N=%d\n", N);
    else if (z < 0)
        printf("Error: parameter 3 (z) should be non negative\n");
    else if (z > N)
        printf("Error: parameter 3 (z) should be smaller or equal to N=%d\n", N);
    else if (game->board->board_mat[y - 1][x - 1].type == FIXED)
        printf("Error: can't change the value of a fixed cell\n");
    else {
        old_val = game->board->board_mat[y - 1][x - 1].val;
        set(game->board, y - 1, x - 1, z);
        act_list = malloc(3 * sizeof(int));
        if (act_list == NULL)
            func_fail_exit("malloc");
        act_list[0] = y - 1;
        act_list[1] = x - 1;
        act_list[2] = old_val;
        list_add(game->list, 1, act_list);
        user_print_board(game);
        check_game_end(game);
    }
}

void validate(game_t *game) {
    board_t *copy;
    solve_status_t status;
    if (game->board->error_num > 0)
        printf("Error: validate can't be use if the board is erroneous\n");
    else {
        copy = copy_board(game->board);
        status = optimized_solve(copy, ILP, NULL);
        switch (status) {
            case SOLVE_SUCCESS:
                printf("Board is solvable\n");
                break;
            case SOLVE_FAIL:
                printf("Board is unsolvable\n");
                break;
            case GRB_FAIL:
                printf("Error: validation failed due to gurobi error\n");
                break;
        }
        board_dtor(copy);
    }
}

void hint(game_t *game, int x, int y) {
    int N = game->board->N;
    board_t *copy;
    solve_status_t status;
    int i = y - 1, j = x - 1;
    if (x < 1)
        printf("Error: parameter 1 (x) should be positive\n");
    else if (x > N)
        printf("Error: parameter 1 (x) should be smaller or equal to N=%d\n", N);
    else if (y < 1)
        printf("Error: parameter 2 (y) should be positive\n");
    else if (y > N)
        printf("Error: parameter 2 (y) should be smaller or equal to N=%d\n", N);
    else if (game->board->error_num > 0)
        printf("Error: the hint can't be used if the board is erroneous\n");
    else if (game->board->board_mat[i][j].type == FIXED)
        printf("Error: the hint command can't be used on a fixed cell\n");
    else if (game->board->board_mat[i][j].val != EMPTY_CELL)
        printf("Error: the hint command can't be used on a non empty cell\n");
    else {
        copy = copy_board(game->board);
        status = optimized_solve(copy, ILP, NULL);
        switch (status) {
            case SOLVE_SUCCESS:
                printf("hint: set cell (%d,%d) to %d\n", x, y, copy->board_mat[i][j].val);
                break;
            case SOLVE_FAIL:
                printf("Error: board is unsolvable\n");
                break;
            case GRB_FAIL:
                printf("Error: failed to find a hint do to gurobi error\n");
                break;
        }
        board_dtor(copy);
    }
}

/*Helper for generate, after the call the first
 * k elements of list will be chosen uniformly from all
 * of the first len element of list.*/
static void k_shuffle(int *list, int k, int len) {
    int i, j, tmp;
    for (i = 0; i < k; ++i) {
        j = rand() % (len - i) + i;
        tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
    }
}

/*Helper for generate, inacts iteration of generate,
 * emp_arr is array of one number indexs of the empty cells of the board
 * vals is buff it length N+1 and x is the x from generate.
 * returns the return status of ILP solver that it uses on the board.*/
static solve_status_t generate_iter(board_t *board, int *emp_arr, int *vals, int x) {
    int i, N = board->N;
    k_shuffle(emp_arr, x, N * N - board->filled_num);
    for (i = 0; i < x; ++i) {
        possible_val_list(board, vals, emp_arr[i] / N, emp_arr[i] % N);
        if (vals[0] == 0)
            return SOLVE_FAIL;
        set(board, emp_arr[i] / N, emp_arr[i] % N, vals[1 + rand() % vals[0]]);
    }
    switch (optimized_solve(board, ILP, NULL)) {
        case SOLVE_SUCCESS:
            return SOLVE_SUCCESS;
        case SOLVE_FAIL:
            return SOLVE_FAIL;
        case GRB_FAIL:
            return GRB_FAIL;
        default:
            assert(0);/*should not happen*/
    }
}

/*Helper for generate, cleanly changes the board from orig
 * to next and updates the undo/redo list*/
static void change_board(game_t *game, board_t *orig, board_t *next) {
    int i, j, act_num = 0, *act_arr = NULL, N = orig->N;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (orig->board_mat[i][j].val != next->board_mat[i][j].val)
                ++act_num;
        }
    }
    if (act_num != 0) {
        act_arr = malloc(3 * act_num * sizeof(int));
        if (act_arr == NULL)
            func_fail_exit("malloc");
        act_num = 0;
        for (i = 0; i < N; ++i) {
            for (j = 0; j < N; ++j) {
                if (orig->board_mat[i][j].val != next->board_mat[i][j].val) {
                    act_arr[3 * act_num] = i;
                    act_arr[3 * act_num + 1] = j;
                    act_arr[3 * act_num + 2] = orig->board_mat[i][j].val;
                    ++act_num;
                }
            }
        }
    }
    list_add(game->list, act_num, act_arr);
    board_dtor(orig);
    game->board = next;
}

void generate(game_t *game, int x, int y) {
    int i, iter, *emp_arr, *vals, *cell_arr;
    board_t *board = game->board, *copy;
    int N = board->N;
    int e = N * N - board->filled_num;
    solve_status_t s;
    if (x < 0)
        printf("Error: parameter 1 (x) should not be negative\n");
    else if (y < 0)
        printf("Error: parameter 2 (y) should not be negative\n");
    else if (x > e)
        printf("Error: parameter 1 (x) is too large, there are only %d empty cells in the board\n", e);
    else if (y > N * N)
        printf("Error: parameter 2 (y) is too large, there are only %d cells in the board\n", N * N);
    else {
        copy = copy_board(board);
        emp_arr = empty_cell_list(copy);
        vals = (int *) malloc((N + 1) * sizeof(int));
        if (vals == NULL)
            func_fail_exit("malloc");
        for (iter = 0; iter < MAX_GEN_ITER; ++iter) {
            s = generate_iter(copy, emp_arr, vals, x);
            if (s != SOLVE_FAIL)
                break;
            for (i = 0; i < e; ++i)
                set(board, emp_arr[i] / N, emp_arr[i] % N, 0);
        }
        free(vals);
        free(emp_arr);
        if (s != SOLVE_SUCCESS) {
            if (s == SOLVE_FAIL)
                printf("Error: could not generate board, maximum number of iterations reached\n");
            if (s == GRB_FAIL)
                printf("Error: could not complete generate command do to gurobi error\n");
            board_dtor(copy);
            return;
        }
        cell_arr = (int *) malloc(N * N * sizeof(int));
        if (cell_arr == NULL)
            func_fail_exit("malloc");
        for (i = 0; i < N * N; ++i)
            cell_arr[i] = i;
        k_shuffle(cell_arr, N * N - y, N * N);
        for (i = 0; i < N * N - y; ++i)
            set(copy, cell_arr[i] / N, cell_arr[i] % N, 0);
        free(cell_arr);
        change_board(game, board, copy);
        user_print_board(game);
    }
}

/*Helper for guess gets a array of scores of each possible value
 * of a cell (probs), the possible values of the cell (vals) and thershold(X)
 * and returns a guess to the vaule of the cell or EMPTY_CELL
 * if onne can't be made.*/
static int guess_value(const double *scores, int *vals, double X) {
    int i;
    double sum = 0., r;
    for (i = 1; i <= vals[0]; ++i)
        sum += scores[vals[i]] >= X ? scores[vals[i]] : 0.;
    if (sum == 0.)
        return EMPTY_CELL;
    r = ((double) rand() / RAND_MAX) * sum;
    sum = 0.;
    for (i = 1; i <= vals[0]; ++i) {
        if (scores[vals[i]] != 0.) {
            sum += scores[vals[i]];
            if (sum >= r)
                return vals[i];
        }
    }
    return EMPTY_CELL;
}

/*gets the game a matrix of scores (in the format of the LP solver)
 * and threshold X and enacts the guess on the board assuming
 * the operation is valid*/
static void enact_guess(game_t *game, double *score_mat, double X) {
    int i, j, g, act_num = 0, *vals, *act_arr;
    board_t *board = game->board;
    int N = board->N;
    vals = malloc((N + 1) * sizeof(int));
    if (vals == NULL)
        func_fail_exit("malloc");
    act_arr = malloc((3 * N * N) * sizeof(int));
    if (act_arr == NULL)
        func_fail_exit("malloc");
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (board->board_mat[i][j].val == EMPTY_CELL) {
                possible_val_list(board, vals, i, j);
                g = guess_value(score_mat + N * N * i + N * j, vals, X);
                if (g != EMPTY_CELL) {
                    act_arr[3 * act_num] = i;
                    act_arr[3 * act_num + 1] = j;
                    act_arr[3 * act_num + 2] = EMPTY_CELL;
                    set(board, i, j, g);
                    ++act_num;
                }
            }
        }
    }
    free(vals);
    if (act_num == 0) {
        free(act_arr);
        act_arr = NULL;
    } else {
        act_arr = realloc(act_arr, 3 * act_num * sizeof(int));
        if (act_arr == NULL)
            func_fail_exit("realloc");
    }
    list_add(game->list, act_num, act_arr);
}

void guess(game_t *game, double X) {
    int N = game->board->N;
    solve_status_t status;
    double *score_mat;
    board_t *copy;
    if (X < 0. || X > 1.)
        printf("Error: parameter 1 (x) should be between 0 and 1\n");
    else if (game->board->error_num > 0)
        printf("Error: the guess command can't be used if the board is erroneous\n");
    else {
        score_mat = (double *) calloc(N * N * N, sizeof(double));
        if (score_mat == NULL)
            func_fail_exit("calloc");
        copy = copy_board(game->board);
        status = optimized_solve(copy, LP, score_mat);
        board_dtor(copy);
        switch (status) {
            case SOLVE_SUCCESS:
                enact_guess(game, score_mat, X);
                break;
            case SOLVE_FAIL:
                printf("Error: board is unsolvable\n");
                break;
            case GRB_FAIL:
                printf("Error: failed to complete the guess command do to gurobi error\n");
                break;
        }
        free(score_mat);
        user_print_board(game);
        check_game_end(game);
    }
}

void guess_hint(game_t *game, int x, int y) {
    board_t *copy;
    double *score_mat, *scores;
    solve_status_t status;
    int k, i = y - 1, j = x - 1;
    int N = game->board->N;
    if (x < 1)
        printf("Error: parameter 1 (x) should be positive\n");
    else if (x > N)
        printf("Error: parameter 1 (x) should be smaller or equal to N=%d\n", N);
    else if (y < 1)
        printf("Error: parameter 2 (y) should be positive\n");
    else if (y > N)
        printf("Error: parameter 2 (y) should be smaller or equal to N=%d\n", N);
    else if (game->board->error_num > 0)
        printf("Error: the guess_hint command can't be used if the board is erroneous\n");
    else if (game->board->board_mat[i][j].type == FIXED)
        printf("Error: the guess_hint command can't be used on a fixed cell\n");
    else if (game->board->board_mat[i][j].val != EMPTY_CELL)
        printf("Error: the guess_hint command can't be used on a non empty cell\n");
    else {
        score_mat = (double *) calloc(N * N * N, sizeof(double));
        if (score_mat == NULL)
            func_fail_exit("calloc");
        copy = copy_board(game->board);
        status = optimized_solve(copy, LP, score_mat);
        board_dtor(copy);
        scores = score_mat + N * N * i + N * j;
        switch (status) {
            case SOLVE_SUCCESS:
                for (k = 0; k < N; ++k) {
                    if (scores[k] != 0.)
                        printf("value %d score is %f\n", k + 1, scores[k]);
                }
                break;
            case SOLVE_FAIL:
                printf("Error: board is unsolvable\n");
                break;
            case GRB_FAIL:
                printf("Error: failed to find a guess_hint do to gurobi error\n");
                break;
        }
        free(score_mat);
    }
}

void num_sol(game_t *game) {
    board_t *copy;
    if (game->board->error_num > 0)
        printf("Error: num_solutions can't be use if the board is erroneous\n");
    else {
        copy = copy_board(game->board);
        printf("The board has %d solutions\n", count_sol(copy));
        board_dtor(copy);
    }
}

/*Helper to autofill, reinserts TEMP cells as UESR cells
 * (make them consistent with the board) and update undo/redo list
 * accordingly (act_list is pre allocated buffer in the needed size).*/
static void persist_temps_and_update(board_t *board, int *act_list, int N) {
    int i, j, z, c = 0;
    entry_t **mat = board->board_mat;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (mat[i][j].type == TEMP) {
                z = mat[i][j].val;
                mat[i][j].val = EMPTY_CELL;
                mat[i][j].type = EMPTY;
                set(board, i, j, z);
                act_list[3 * c] = i;
                act_list[3 * c + 1] = j;
                act_list[3 * c + 2] = EMPTY_CELL;
                ++c;
            }
        }
    }
}

void autofill(game_t *game) {
    int i, j, *vals, num_of_actions = 0, *act_list = NULL;
    board_t *board = game->board;
    int N = board->N;
    entry_t **mat = board->board_mat;
    if (board->error_num > 0)
        printf("Error: autofill can't be use if the board is erroneous\n");
    else {
        vals = malloc((N + 1) * sizeof(int));
        if (vals == NULL)
            func_fail_exit("malloc");
        for (i = 0; i < N; ++i) {
            for (j = 0; j < N; ++j) {
                if (mat[i][j].val == EMPTY_CELL) {
                    possible_val_list(board, vals, i, j);
                    if (vals[0] == 1) {
                        mat[i][j].val = vals[1];
                        mat[i][j].type = TEMP;
                        ++num_of_actions;
                    }
                }
            }
        }
        if (num_of_actions != 0) {
            act_list = malloc(3 * num_of_actions * sizeof(int));
            if (act_list == NULL)
                func_fail_exit("malloc");
            persist_temps_and_update(board, act_list, N);
        }
        list_add(game->list, num_of_actions, act_list);
        free(vals);
        user_print_board(game);
        check_game_end(game);
    }
}

void undo(game_t *game) {
    list_t *list = game->list;
    if (list->current->num_of_act == LIST_START)
        printf("Error: no action to undo\n");/*at the start of the game see list_ctor at list.h*/
    else {
        enact(game->board, list->current, PRINT_OUTPUT);
        list->current = list->current->prev;
        user_print_board(game);
        check_game_end(game);
    }
}

void redo(game_t *game) {
    list_t *list = game->list;
    if (list->current->next == NULL)
        printf("Error: no action to redo\n");
    else {
        list->current = list->current->next;
        enact(game->board, list->current, PRINT_OUTPUT);
        user_print_board(game);
        check_game_end(game);
    }
}

void reset(game_t *game) {
    list_t *list = game->list;
    while (list->current->prev != NULL) {
        enact(game->board, list->current, DONT_PRINT);
        list->current = list->current->prev;
    }
    user_print_board(game);
    check_game_end(game);
}

void game_exit(game_t *game) {
    game_dtor(game);
    printf("Exiting...\n");
    exit(0);
}
