#include <string.h>
#include <assert.h>
#include "parser.h"

/* string to command_t dictionary */
static const command_info_t *str_to_command_info(char *str) {
    static const command_info_t commands[] = {
            {"solve",         SOLVE,         1, F_ANY},
            {"edit",          EDIT,          1, F_ANY},
            {"mark_errors",   MARK_ERRORS,   1, F_SOLVE},
            {"print_board",   PRINT_BOARD,   0, F_E_OR_S},
            {"set",           SET,           3, F_E_OR_S},
            {"validate",      VALIDATE,      0, F_E_OR_S},
            {"guess",         GUESS,         1, F_SOLVE},
            {"generate",      GENERATE,      2, F_EDIT},
            {"undo",          UNDO,          0, F_E_OR_S},
            {"redo",          REDO,          0, F_E_OR_S},
            {"save",          SAVE,          1, F_E_OR_S},
            {"hint",          HINT,          2, F_SOLVE},
            {"guess_hint",    GUESS_HINT,    2, F_SOLVE},
            {"num_solutions", NUM_SOLUTIONS, 0, F_E_OR_S},
            {"autofill",      AUTOFILL,      0, F_SOLVE},
            {"reset",         RESET,         0, F_E_OR_S},
            {"exit",          EXIT,          0, F_ANY},
            {"none",          NONE,          0, F_ANY}
    };
    int i;
    const command_info_t *command_info;
    if (str == NULL)
        return &commands[NONE];
    for (i = 0; i < COMMAND_NUM; ++i) {
        command_info = &commands[i];
        if (strcmp(str, command_info->command_name) == 0)
            return command_info;
    }
    return NULL;
}

parse_status_t parse_command(command_with_args_t *command_with_args, char *str, char *missing_parameter, int mode) {
    int i;
    char *word = strtok(str, " \t");
    const command_info_t *command_info = str_to_command_info(word);
    if (command_info == NULL)
        return PARSE_FAIL_NAME;
    if (!((command_info->mode_mask) & mode)) {
        command_with_args->info = command_info;
        return PARSE_FAIL_WRONG_MODE;
    }
    for (i = 0; i < command_info->arg_num; ++i) {
        command_with_args->args[i] = strtok(NULL, " \t");
        if (command_with_args->args[i] == NULL && command_info->command_type != EDIT) {
            switch (i) {
                case 0:
                    missing_parameter[0] = '1';
                    missing_parameter[1] = 'x';
                    break;
                case 1:
                    missing_parameter[0] = '2';
                    missing_parameter[1] = 'y';
                    break;
                case 2:
                    missing_parameter[0] = '3';
                    missing_parameter[1] = 'z';
                    break;
                default:
                    assert(0); /* should not get here */
            }

            return PARSE_FAIL_FEW_ARGS;
        }
    }
    if (strtok(NULL, " \t") != NULL)
        return PARSE_FAIL_MANY_ARGS;

    command_with_args->info = command_info;
    return PARSE_SUCCESS;
}




