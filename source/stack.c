#include <limits.h>
#include <stdlib.h>

#include "stack.h"

#define STACK_ERR (void *)LONG_MIN

_stack_t *stack_new(void){
    _stack_t *stack = malloc(sizeof(_stack_t));

    stack->data = NULL;
    stack->top = -1;

    return stack;
}

void stack_push(_stack_t *stack, void *data){
    if(!stack)
        return;

    void **data_rea = realloc(stack->data, sizeof(void *) * (++stack->top + 1));

    stack->data = data_rea;
    stack->data[stack->top] = data;
}

void *stack_pop(_stack_t *stack){
    if(stack_empty(stack))
        return STACK_ERR;

    return stack->data[stack->top--];
}

void *stack_peek(_stack_t *stack){
    if(stack_empty(stack))
        return STACK_ERR;
    
    return stack->data[stack->top];
}

int stack_empty(_stack_t *stack){
    if(!stack)
        return 1;

    if(!stack->data)
        return 1;

    return stack->top == -1;
}

void stack_free(_stack_t *stack){
    if(!stack)
        return;

    free(stack->data);
    free(stack);

    stack->data = NULL;
    stack = NULL;
}
