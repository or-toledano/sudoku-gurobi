#include <stdlib.h>
#include "board.h"
#include "stack.h"
#include "main_aux.h"
#include "solution_counter.h"

/*Destructor for call_t*/
void call_dtor(void *call) {
    free((call_t *) call);
}

/*Pushes the call needed of cell i,j in the current state of board
 * and return the number of item pushed.
 * the first value of val_and_arg_arr is the index of cell i,j in the empty
 * cell list.*/
int push_calls(board_t *board, stack_t *stack, int *val_and_arg_arr, int i, int j) {
    int k;
    int c = val_and_arg_arr[0];
    int *buff = val_and_arg_arr + 1;
    call_t *call;
    possible_val_list(board, buff, i, j);
    if (!buff[0])
        return 0;
    for (k = 1; k <= buff[0]; ++k) {
        call = (call_t *) malloc(sizeof(call_t));
        if (call == NULL)
            func_fail_exit("malloc");
        call->c = c;
        call->z = buff[k];
        stack_push(stack, (void *) call);
    }
    return buff[0];
}

int count_sol(board_t *board) {
    int i;
    int c = 0;
    int pc = 0;
    int res = 0;
    int N = board->N;
    int empty_num = N * N - board->filled_num;
    int *empty_list;
    int *val_arr;
    call_t *call;
    node_t *node;
    stack_t *stack;
    if (empty_num == 0)
        return 1;
    empty_list = empty_cell_list(board);
    stack = stack_ctor();
    val_arr = (int *) malloc((N + 2) * sizeof(int));
    if (val_arr == NULL)
        func_fail_exit("malloc");
    val_arr[0] = c;
    if (push_calls(board, stack, val_arr, empty_list[c] / N, empty_list[c] % N) == 0) {
        free(empty_list);
        free(val_arr);
        free(stack);
        return 0;
    }
    while (is_empty(stack) == NOT_EMPTY) {
        node = stack_pop(stack);
        call = (call_t *) (node->value);
        c = call->c;
        if (pc > c) {
            for (i = c; i < pc; ++i) {
                set(board, empty_list[i] / N, empty_list[i] % N, 0);
            }
        }
        if (c == empty_num - 1)
            ++res;
        else {
            set(board, empty_list[c] / N, empty_list[c] % N, call->z);
            ++c;
            val_arr[0] = c;
            push_calls(board, stack, val_arr, empty_list[c] / N, empty_list[c] % N);
        }
        pc = c;
        node_dtor(node, call_dtor);
    }
    free(empty_list);
    free(val_arr);
    free(stack);
    return res;
}
