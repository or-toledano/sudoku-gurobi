#include <stdlib.h>
#include "stack.h"
#include "main_aux.h"

/*constructor for node_t*/
static node_t *node_ctor(void *value) {
    node_t *node = malloc(sizeof(node_t));
    if (node == NULL)
        func_fail_exit("malloc");
    node->value = value;
    node->prev = NULL;
    return node;
}

void node_dtor(node_t *node, void (*value_dtor)(void *)) {
    value_dtor(node->value);
    free(node);
}


stack_t *stack_ctor() {
    stack_t *stack = malloc(sizeof(stack_t));
    if (stack == NULL)
        func_fail_exit("malloc");
    stack->top = NULL;
    return stack;
}


node_t *stack_pop(stack_t *stack) {
    node_t *top = stack->top;
    if (top != NULL) {
        stack->top = top->prev;
        top->prev = NULL;
    }
    return top;
}

/*Push node to stack*/
static void stack_push_node(stack_t *stack, node_t *node) {
    if (stack->top == NULL)
        stack->top = node;
    else {
        node->prev = stack->top;
        stack->top = node;
    }
}


void stack_push(stack_t *stack, void *value) {
    node_t *node = node_ctor(value);
    stack_push_node(stack, node);
}

int is_empty(stack_t *stack) {
    if (stack->top == NULL)
        return STACK_EMPTY;
    else
        return NOT_EMPTY;
}

void stack_dtor(stack_t *stack, void (*value_dtor)(void *)) {
    node_t *del_node;
    if (stack->top != NULL) {
        while (stack->top->prev != NULL) {
            del_node = stack_pop(stack);
            stack->top = stack->top->prev;
            node_dtor(del_node, value_dtor);
        }
        node_dtor(stack->top, value_dtor);
    }
    free(stack);
}




