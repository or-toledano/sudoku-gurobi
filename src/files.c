#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "game.h"

/*Tries to exit cleanly from file write handler if an error occurred proper error message will be printed*/
static void exit_fhw(FILE *f) {
    printf("Error: failed writing to file,%s\n", strerror(errno));
    if (fclose(f) < 0)
        printf("Error: can't close file, %s\n", strerror(errno));
}

void save_to_file(board_t *board, const char *file_name, entry_type_t user_cell) {
    int i;
    int j;
    int n = board->n;
    int m = board->m;
    int N = n * m;
    entry_t **mat = board->board_mat;
    FILE *f = fopen(file_name, "w");
    if (f == NULL)
        printf("Error: can't open file, %s\n", strerror(errno));
    if (fprintf(f, "%d %d\n", m, n) < 0) {
        exit_fhw(f);
        return;
    }
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (fprintf(f, "%d", mat[i][j].val) < 0) {
                exit_fhw(f);
                return;
            }
            if ((user_cell == FIXED && mat[i][j].val != 0) || mat[i][j].type == FIXED)
                fprintf(f, ".");
            if (j == N - 1)
                fprintf(f, "\n");
            else
                fprintf(f, " ");
        }
    }
    if (fclose(f) < 0)
        printf("Error: can't close file, %s\n", strerror(errno));
}

/*Tries to exit cleanly from file read handler if an error occurred proper error message will be printed*/
static board_t *exit_fhr(FILE *f, board_t *board, board_t *fixed_board, const char *msg) {
    printf("%s", msg);
    if (board != NULL)
        board_dtor(board);
    if (fixed_board)
        board_dtor(fixed_board);
    if (fclose(f) < 0)
        printf("Error: can't close file, %s\n", strerror(errno));
    return NULL;
}

/* Checks what error occurred during the reading of the file
 * and tries to exit cleanly printing error messages along the way
 * ret is the return value of the failed library function
 * and char_read is the char read if any was read (EOF is none was read)*/
static board_t *check_read_error_and_exit(FILE *f, board_t *board, board_t *fixed_board, int ret, int char_read) {
    int c;
    if (feof(f) && char_read == EOF)
        printf("Error: not enough values in file\n");
    else if (ret == EOF)
        printf("Error: error while reading file, %s\n", strerror(errno));
    else {
        if (char_read == EOF) {
            c = fgetc(f);
            if (c == EOF)
                printf("Error: error while reading file, %s\n", strerror(errno));
        } else
            c = char_read;
        if (c != EOF)
            printf("Error: unexpected character, '%c', in file\n", (char) c);
    }
    return exit_fhr(f, board, fixed_board, "");
}

board_t *read_from_file(const char *file_name, entry_type_t fixed_cell) {
    int i, m, n, val, N, ret, c;
    char cc;
    board_t *board = NULL, *fixed_board = NULL;
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        printf("Error: can't open file, %s\n", strerror(errno));
        return NULL;
    }
    if ((ret = fscanf(f, "%d %d", &m, &n)) < 2)
        return check_read_error_and_exit(f, board, fixed_board, ret, EOF);
    if (m < 1) {
        printf("Error: m read from file is smaller then 1\n");
        return NULL;
    }
    if (n < 1) {
        printf("Error: n read from file is smaller then 1\n");
        return NULL;
    }
    board = board_ctor(n, m);
    N = n * m;
    for (i = 0; i < N * N; ++i) {
        if ((ret = fscanf(f, "%d", &val)) < 1) {
            check_read_error_and_exit(f, board, fixed_board, ret, EOF);
            return NULL;
        }
        if (val > N || val < 0) {
            printf("Error: read cell value from file that is not between 0 and N=%d\n", N);
            return NULL;
        }
        set(board, i / N, i % N, val);
        if (feof(f)) {
            if (i == N * N - 1)
                break;
        }
        c = fgetc(f);
        if (c == '.') {
            if (fixed_cell == FIXED) {
                board->board_mat[i / N][i % N].type = FIXED;
                if (val == 0)
                    return exit_fhr(f, board, fixed_board, "Error: empty cell can not be fixed\n");
                if (fixed_board == NULL)
                    fixed_board = board_ctor(n, m);
                set(fixed_board, i / N, i % N, val);
                if (fixed_board->error_num > 0)
                    return exit_fhr(f, board, fixed_board, "Error: erroneous fixed cell in file\n");
            }
        } else if (!isspace(c))
            return check_read_error_and_exit(f, board, fixed_board, c, c);
    }
    if (fscanf(f, " %c", &cc) > 0)
        return exit_fhr(f, board, fixed_board, "Error: file too long\n");
    else if (!feof(f)) {
        printf("Error: error while reading file, %s\n", strerror(errno));
        return exit_fhr(f, board, fixed_board, "");
    }
    board_dtor(fixed_board);
    if (fclose(f) < 0)
        printf("Error: can't close file, %s\n", strerror(errno));
    return board;
}
