#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdbool.h>

#define STACK_INITIAL_CAPACITY 16
#define STACK_GROW_FACTOR 2


typedef struct _Stack {
    void **items;
    size_t size;
    size_t capacity;
} Stack;

void Stack_Init(Stack *stack);
void Stack_Deinit(Stack *stack);

void Stack_Push(Stack *stack, void *item);
void *Stack_Pop(Stack *stack);
void *Stack_Peek(const Stack *stack);

bool Stack_Has(const Stack *stack, const void *item);

void Stack_Clear(Stack *stack);


#endif