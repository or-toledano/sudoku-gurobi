/* board module - defines the board with its states, and the functions of the board */
#ifndef SUDOKU_PROJECT_BOARD_H
#define SUDOKU_PROJECT_BOARD_H

/*Remark about cell indexing:
 * x,y indexing - cell x,y refer to the cell if column x and row y
 *  where index ard 1-based (used in game only)
 * i,j indexing - cell i,j refers to cell in row i and column j
 *  where index ard 0-based
 * one number indexing - cell i,j in i,j indexing is cell i*N+j in one number indexing*/

/* Macros for printing */
#define SPACES "    "
#define SEPERATOR "|"
#define H_SEPARATOR "-"
/* Macros for functions output*/
#define SAFE 0
#define ERR 1

/*Macro for entry representation*/
#define EMPTY_CELL 0

/*Macro for entry representation*/
#define EMPTY_CELL 0


/*entry_t Struct*/

/*EMPTY - empty entry
 *FIXED - fixed entry
 *TEMP - an entry that does not (necessarily) consistent
 * with the current state of the board
 *USER - user or user like cell, more
 * generally cell that are surly consistent with board state.*/
typedef enum {
    EMPTY = 0, FIXED, TEMP, USER
} entry_type_t;

typedef struct entry_s {
    int val;/*The value of the entry*/
    entry_type_t type;/*The type of the entry*/
} entry_t;

/* Struct : board_t describes a game board with some information
 * row/col/block_counter[i][j] will contain the number of 
 * appearances of j in row/column/block i.
 * entry_mat[i][j] is the entry of the board in row i and column j.
 */
typedef struct {
    entry_t **board_mat;
    int **row_counter;
    int **col_counter;
    int **block_counter;
    int n;
    int m;
    int N; /* n*m */
    int error_num;/*# of errors in the board error multiple error in same cell count*/
    int filled_num;/*# of non empty cells*/
} board_t;

/* Checks if value z for cell i,j is erroneous*/
int is_erroneous_value(board_t *board, int i, int j, int z);

/*returns list of empty cell indexes in one number indexing*/
int *empty_cell_list(board_t *board);

/*prints the board mark error if mark_errors=1 and does not
 * if mark_errors=0.*/
void print_board(board_t *board, int mark_errors);

/*after the use of the function the first element of buff is the number of possible values
of the i,j cell in game_board and the buff[0] next elements are the possible values*/
void possible_val_list(board_t *board, int *buff, int i, int j);

/* Sets the value z in cell i,j assuming the set is valid
 * if z!=0 the type of the cell set to USER otherwise it set
 * to EMPTY.*/
void set(board_t *board, int i, int j, int z);

/*Construct empty board with the given n,m*/
board_t *board_ctor(int n, int m);

/*Destructor for board_t*/
void board_dtor(board_t *board);

/*Deep-copys the board*/
board_t *copy_board(board_t *board);


#endif
