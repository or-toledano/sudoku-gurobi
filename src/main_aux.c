#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_aux.h"
#include "parser.h"


#define MAX_CHARS 256

void func_fail_exit(const char *msg) {
    printf("Error: %s has failed\n", msg);
    exit(EXIT_FAILURE);
}

/*Covert_to_double - cover the string arg
 * to  a double and save it into ret if
 * error occurs proper error message is printed and FAILURE
 * returned otherwise SUCCESS is returned.*/
static int covert_to_double(char *arg, double *ret) {
    char *e;
    *ret = strtod(arg, &e);
    if (*e != '\0') {
        printf("Error parameter 1 (x) should be a floating point number\n");
        return FAILURE;
    }
    return SUCCESS;
}

/*Covert_to_int - cover the first arg_num strings at arg
 * to a int and save them it into the first arg_num ints at
 * ret (at the same order).
 * if error occurs proper error message is printed and FAILURE
 * returned otherwise SUCCESS is returned.*/
static int covert_to_int(char *args[], int ret[], int arg_num) {
    int i;
    char *e, sym[] = {'x', 'y', 'z'};
    for (i = 0; i < arg_num; ++i) {
        ret[i] = strtol(args[i], &e, 10);
        if (*e != '\0') {
            printf("Error parameter %d (%c) should be an integer\n", i + 1, sym[i + 1]);
            return FAILURE;
        }
    }
    return SUCCESS;
}

/*Dispatch_command - calls the game function that
 * inacts the command parameter if an error occurs
 * a proper error message is printed.*/
static void dispatch_command(game_t *game, command_with_args_t *command) {
    int int_args[3];
    double d_arg;
    switch (command->info->command_type) {
        case SOLVE:
            solve(game, command->args[0]);
            break;
        case EDIT:
            edit(game, command->args[0]);
            break;
        case MARK_ERRORS:
            if (covert_to_int(command->args, int_args, command->info->arg_num) == SUCCESS)
                mark_errors(game, int_args[0]);
            break;
        case PRINT_BOARD:
            user_print_board(game);
            break;
        case SET:
            if (covert_to_int(command->args, int_args, command->info->arg_num) == SUCCESS)
                user_set(game, int_args[0], int_args[1], int_args[2]);
            break;
        case VALIDATE:
            validate(game);
            break;
        case GUESS:
            if (covert_to_double(command->args[0], &d_arg) == SUCCESS)
                guess(game, d_arg);
            break;
        case GENERATE:
            if (covert_to_int(command->args, int_args, command->info->arg_num) == SUCCESS)
                generate(game, int_args[0], int_args[1]);
            break;
        case UNDO:
            undo(game);
            break;
        case REDO:
            redo(game);
            break;
        case SAVE:
            save(game, command->args[0]);
            break;
        case HINT:
            if (covert_to_int(command->args, int_args, command->info->arg_num) == SUCCESS)
                hint(game, int_args[0], int_args[1]);
            break;
        case GUESS_HINT:
            if (covert_to_int(command->args, int_args, command->info->arg_num) == SUCCESS)
                guess_hint(game, int_args[0], int_args[1]);
            break;
        case NUM_SOLUTIONS:
            num_sol(game);
            break;
        case AUTOFILL:
            autofill(game);
            break;
        case RESET:
            reset(game);
            break;
        case EXIT:
            free(command);
            game_exit(game);
            break;
        case NONE: /* do nothing */
            break;
    }
}


void parse_and_dispatch_commands(game_t *game) {
    int c, end;
    command_with_args_t *command = (command_with_args_t *) malloc(sizeof(command_with_args_t));
    parse_status_t parse_status;
    char str[MAX_CHARS + 2] = {0}, missing_parameter[2]; /* +1 for the terminator +1 for new line*/
    if (!command)
        func_fail_exit("malloc");
    while (1) { /* get the commands until a proper command is inserted */
        printf("Enter command:\n");
        if (fgets(str, MAX_CHARS + 2, stdin) == NULL) {
            if (feof(stdin)) {
                free(command);
                game_exit(game);
            }
            func_fail_exit("fgets");
        }
        end = strcspn(str, "\n");
        if (end > MAX_CHARS) {
            printf("ERROR: command is to long\n");
            for (c = getchar(); c != '\n'; c = getchar())
                if (c == EOF)
                    func_fail_exit("get_char");
            continue;
        }
        str[end] = '\0';
        parse_status = parse_command(command, str, missing_parameter, game->mode);
        if (parse_status == PARSE_FAIL_WRONG_MODE) {
            printf("Error: the %s command is only available in ", command->info->command_name);
            switch (command->info->mode_mask) {
                case F_SOLVE:
                    printf("solve mode\n");
                    break;
                case F_EDIT:
                    printf("edit mode\n");
                    break;
                case F_E_OR_S:
                    printf("solve or edit modes\n");
                    break;
            }
        } else if (parse_status == PARSE_FAIL_MANY_ARGS)
            printf("Error: too many parameters: maximum of %d for %s command\n",
                   command->info->arg_num, command->info->command_name);
        else if (parse_status == PARSE_FAIL_NAME)
            printf("Error: invalid command\n");
        else if (parse_status == PARSE_FAIL_FEW_ARGS)
            printf("Error: parameter %c missing (%c)\n", missing_parameter[0], missing_parameter[1]);
        else
            dispatch_command(game, command);
    }
}


