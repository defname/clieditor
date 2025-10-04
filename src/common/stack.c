#include "stack.h"
#include "logging.h"

static void increase_capacity(Stack *stack) {
    if (!stack) {
        return;
    }
    if (stack->capacity == 0) {
        stack->capacity = STACK_INITIAL_CAPACITY;
    }
    else {
        stack->capacity *= STACK_GROW_FACTOR;
    }
    stack->items = realloc(stack->items, stack->capacity * sizeof(void *));
    if (!stack->items) {
        logFatal("Failed to reallocate stack items.");
    }
}

void Stack_Init(Stack *stack) {
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
    increase_capacity(stack);
}

void Stack_Deinit(Stack *stack) {
    if (!stack) {
        return;
    }
    free(stack->items);
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

void Stack_Push(Stack *stack, void *item) {
    if (!stack) {
        return;
    }
    if (stack->size >= stack->capacity) {
        increase_capacity(stack);
    }
    stack->items[stack->size] = item;
    stack->size++;
}

void *Stack_Pop(Stack *stack) {
    if (!stack || stack->size == 0) {
        return NULL;
    }
    return stack->items[--stack->size];
}

void *Stack_Peek(const Stack *stack) {
    if (!stack || stack->size == 0) {
        return NULL;
    }
    return stack->items[stack->size - 1];
}

bool Stack_Has(const Stack *stack, const void *item) {
    if (!stack) {
        return false;
    }
    for (size_t i=0; i<stack->size; i++) {
        if (stack->items[i] == item) {
            return true;
        }
    }
    return false;
}

void Stack_Clear(Stack *stack) {
    if (!stack) {
        return;
    }
    stack->size = 0;
}
