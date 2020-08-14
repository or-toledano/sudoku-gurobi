/* parser module - does parsing for main_aux */
#ifndef SUDOKU_PROJECT_PARSER_H
#define SUDOKU_PROJECT_PARSER_H

#define COMMAND_NUM 17
#define MAX_ARG_NUM 3
/* not including NONE command */
/*Macros for mode flags and masks*/
#define F_INIT 1
#define F_SOLVE 2
#define F_EDIT 4
#define F_E_OR_S 6/*edit or solve modes*/
#define F_ANY 7/*any mode*/
/* command_t - describes the parsed command type */
typedef enum {
    SOLVE,
    EDIT,
    MARK_ERRORS,
    PRINT_BOARD,
    SET,
    VALIDATE,
    GUESS,
    GENERATE,
    UNDO,
    REDO,
    SAVE,
    HINT,
    GUESS_HINT,
    NUM_SOLUTIONS,
    AUTOFILL,
    RESET,
    EXIT,
    NONE /* do nothing command - when receives spaces and such */
} command_t;

/* command_info_t - describes the parsed command name + type + number of arguments */
typedef struct {
    char *command_name;
    command_t command_type;
    int arg_num; /* Natural number */
    int mode_mask;/*mask used to verify if the command is proper a game mode*/
} command_info_t;


/* Parse status enum */
typedef enum {
    PARSE_SUCCESS, PARSE_FAIL_NAME, PARSE_FAIL_FEW_ARGS, PARSE_FAIL_MANY_ARGS, PARSE_FAIL_WRONG_MODE
} parse_status_t;


/* command_with_args_t - describes the command with arguments*/
typedef struct {
    const command_info_t *info;
    char *args[MAX_ARG_NUM];
} command_with_args_t;


/* Gets the command found in a string into command_with_args, returns the status (was the parse successful)
 * str format:
 * command [arg0] [arg1] [arg2]
 * prints error message and returns a proper parse_status_t
 * NOTE: if there are 0 args of the edit command, then args[0] will be NULL
 * NOTE: the mode parameter given will be given as the flag of the current game mode
*/
parse_status_t parse_command(command_with_args_t *command_with_args, char *str, char missing_parameter[], int mode);

#endif
