#include <stdlib.h>
#include <stdio.h>
#include "board.h"
#include "main_aux.h"


int is_erroneous_value(board_t *board, int i, int j, int z) {
    int res = 0;
    int val = (board->board_mat[i][j]).val - 1;
    int n = board->n;
    int m = board->m;
    int c = z - 1;
    int comp = (val == c);
    res += board->row_counter[i][c] > comp;
    res += board->col_counter[j][c] > comp;
    res += board->block_counter[m * (i / m) + j / n][c] > comp;
    if (res > 0)
        return ERR;
    else
        return SAFE;
}

void possible_val_list(board_t *board, int *buff, int i, int j) {
    int val;
    int N = (board->N);
    buff[0] = 0;
    for (val = 1; val <= N; ++val) {
        if (is_erroneous_value(board, i, j, val) == SAFE) {
            ++buff[0];
            buff[buff[0]] = val;
        }
    }
}

/* Prints a row */
static void print_row(board_t *board, int row, int mark_errors) {
    int j = 0;
    entry_t to_print;
    int n = board->n;
    for (j = 0; j < board->N; ++j) {
        if (j % n == 0)
            printf(SEPERATOR);
        to_print = (board->board_mat)[row][j];
        if (to_print.val == EMPTY_CELL)
            printf(SPACES);
        else if (to_print.type == FIXED)
            printf(" %2d.", to_print.val);
        else if (mark_errors && is_erroneous_value(board, row, j, to_print.val))
            printf(" %2d*", to_print.val);
        else
            printf(" %2d ", to_print.val);
    }
    printf("|\n");
}


void print_board(board_t *board, int mark_errors) {
    int i = 0;
    int j = 0;
    int N = board->N;
    int m = board->m;
    int h_sep_size = 4 * N + m + 1;
    for (i = 0; i < N; ++i) {
        if (i % m == 0) {
            for (j = 0; j < h_sep_size; ++j) {
                printf(H_SEPARATOR);
            }
            printf("\n");
        }
        print_row(board, i, mark_errors);
    }
    for (j = 0; j < h_sep_size; ++j) {
        printf(H_SEPARATOR);
    }
    printf("\n");
}

/* zero the cells of mat */
static void zero_matrix(int **mat, int N) {
    int i, j;
    for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
            mat[i][j] = 0;
}

board_t *copy_board(board_t *board) {
    int i;
    int j;
    int n = board->n;
    int m = board->m;
    board_t *new_board = board_ctor(n, m);
    int N = n * m;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            new_board->board_mat[i][j] = board->board_mat[i][j];
            new_board->row_counter[i][j] = board->row_counter[i][j];
            new_board->col_counter[i][j] = board->col_counter[i][j];
            new_board->block_counter[i][j] = board->block_counter[i][j];
        }
    }
    new_board->error_num = board->error_num;
    new_board->filled_num = board->filled_num;
    return new_board;

}

board_t *board_ctor(int n, int m) {
    int N = n * m;
    entry_t *board_array;
    int i;
    int j;
    int **counters[3];
    board_t *board = (board_t *) malloc(sizeof(board_t));
    board->n = n;
    board->m = m;
    board->N = N;
    board->error_num = 0;
    board->filled_num = 0;
    board->board_mat = (entry_t **) malloc(N * sizeof(entry_t *));
    board_array = (entry_t *) calloc(N * N, sizeof(entry_t));
    for (i = 0; i < N; ++i)
        board->board_mat[i] = board_array + (N * i);
    for (i = 0; i < 3; ++i) {
        counters[i] = (int **) malloc(N * sizeof(int *) + N * N * sizeof(int));
        for (j = 0; j < N; ++j)
            counters[i][j] = (int *) (counters[i] + N) + (N * j);
        zero_matrix(counters[i], N);
    }
    board->row_counter = counters[0];
    board->col_counter = counters[1];
    board->block_counter = counters[2];
    return board;

}

void board_dtor(board_t *board) {
    if (board != NULL) {
        free(board->board_mat[0]);
        free(board->board_mat);
        free(board->block_counter);
        free(board->row_counter);
        free(board->col_counter);
        free(board);
    }
}

void set(board_t *board, int i, int j, int z) {
    int c = z - 1;
    int m = board->m;
    int n = board->n;
    int *rc = board->row_counter[i];
    int *cc = board->col_counter[j];
    int *bc = board->block_counter[m * (i / m) + j / n];
    entry_t old = (board->board_mat)[i][j];

    if (old.val == z) {
        return;
    }
    if (z == EMPTY_CELL) {
        --(board->filled_num);
        (board->board_mat)[i][j].type = EMPTY;
        (board->board_mat)[i][j].val = z;
    } else {
        if (rc[c]++ > 0)
            ++(board->error_num);
        if (cc[c]++ > 0)
            ++(board->error_num);
        if (bc[c]++ > 0)
            ++(board->error_num);
        (board->board_mat)[i][j].val = z;
    }
    if (old.val != 0) {
        if (--rc[old.val - 1] > 0)
            --(board->error_num);
        if (--cc[old.val - 1] > 0)
            --(board->error_num);
        if (--bc[old.val - 1] > 0)
            --(board->error_num);
    } else {
        ++(board->filled_num);
        (board->board_mat)[i][j].type = USER;
    }
}

int *empty_cell_list(board_t *board) {
    int i;
    int j;
    int c = 0;
    int N = board->N;
    int *empty_list;
    int empty_num = N * N - board->filled_num;
    entry_t **mat = board->board_mat;
    if (empty_num == 0)
        return NULL;
    empty_list = (int *) malloc(empty_num * sizeof(int));
    if (empty_list == NULL)
        func_fail_exit("malloc");
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (mat[i][j].val == EMPTY_CELL) {
                empty_list[c] = N * i + j;
                ++c;
            }
        }
    }
    return empty_list;
}
