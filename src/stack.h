/*
 * stack module - a stack (LIFO)
 * Modular, we don't assume the type of the values
 */
#ifndef SUDOKU_PROJECT_STACK_H
#define SUDOKU_PROJECT_STACK_H

/*Macros for return value of is_empty*/
#define STACK_EMPTY 0
#define NOT_EMPTY 1

/* A stack frame */
typedef struct node_s {
    void *value;
    struct node_s *prev;
} node_t;

typedef struct {
    node_t *top;
} stack_t;

stack_t *stack_ctor();

/* node/stack dtor uses the dtor of the value of generic type */

void node_dtor(node_t *node, void (*value_dtor)(void *));

void stack_dtor(stack_t *stack, void (*value_dtor)(void *));

/* insert a new value */
void stack_push(stack_t *stack, void *value);

/* is the stack empty */
int is_empty(stack_t *stack);

/* returns top of stack, removes it from the stack */
node_t *stack_pop(stack_t *stack);


#endif
