#include <stdlib.h>
#include <stdio.h>
#include "board.h"
#include "list.h"
#include "main_aux.h"

/*Constructor for double_node_t, Construct a double_node_t with the given values*/
static double_node_t *double_node_ctor(int num_of_act, int *act_list) {
    double_node_t *node = malloc(sizeof(double_node_t));
    if (node == NULL)
        func_fail_exit("malloc");
    node->num_of_act = num_of_act;
    node->act_list = act_list;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

/*Destructor for double_node_t*/
static void double_node_dtor(double_node_t *node) {
    if (node->act_list != NULL)
        free(node->act_list);
    free(node);
}


void enact(board_t *board, double_node_t *action, to_print_output_t print) {
    int k, i, j, z, old_val;
    int num_of_act = action->num_of_act;
    int *act_list = action->act_list;
    entry_t **mat = board->board_mat;
    if (print == PRINT_OUTPUT && action->num_of_act == 0) {
        printf("No changes where made\n");
        return;
    }
    for (k = 0; k < num_of_act; ++k) {
        i = act_list[3 * k];
        j = act_list[3 * k + 1];
        z = act_list[3 * k + 2];
        old_val = mat[i][j].val;
        set(board, i, j, z);
        act_list[3 * k + 2] = old_val;
        if (print == PRINT_OUTPUT)
            printf("Changed cell (%d,%d) from %d to %d\n", j + 1, i + 1, old_val, z);
    }
}

list_t *list_ctor() {
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL)
        func_fail_exit("malloc");
    list->current = NULL;
    list_add(list, LIST_START, NULL);
    return list;
}


void list_clear_redo(list_t *list) {
    if (list->current != NULL) {
        double_node_t *last_before_redo = list->current, *del_node;
        list->current = list->current->next;
        while (list->current != NULL) {
            del_node = list->current;
            list->current = list->current->next;
            double_node_dtor(del_node);
        }
        last_before_redo->next = NULL;
        list->current = last_before_redo;
    }
}

void list_dtor(list_t *list) {
    double_node_t *del_node;
    if (list == NULL)
        return;
    if (list->current != NULL) {
        /* clear from current to last item */
        list_clear_redo(list);
        /* clear the rest */
        while (list->current->prev != NULL) {
            del_node = list->current;
            list->current = list->current->prev;
            double_node_dtor(del_node);
        }
        double_node_dtor(list->current);
    }
    free(list);
}


void list_add(list_t *list, int num_of_act, int *act_list) {
    double_node_t *new_node;
    list_clear_redo(list);
    new_node = double_node_ctor(num_of_act, act_list);
    if (list->current == NULL)
        list->current = new_node;
    else {
        new_node->prev = list->current;
        list->current->next = new_node;
        list->current = new_node;
    }
}
