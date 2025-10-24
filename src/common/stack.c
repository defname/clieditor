/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "stack.h"
#include <string.h>
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

Stack *Stack_Create() {
    Stack *stack = malloc(sizeof(Stack));
    if (!stack) {
        logFatal("Failed to allocate memory for stack.");
    }
    Stack_Init(stack);
    return stack;
}

void Stack_Destroy(Stack *stack) {
    if (!stack) {
        return;
    }
    Stack_Deinit(stack);
    free(stack);
}

Stack *Stack_Copy(const Stack *src) {
    if (!src) {
        return NULL;
    }
    Stack *copy = malloc(sizeof(Stack));
    if (!copy) {
        logFatal("Failed to allocate memory for stack copy.");
    }

    size_t cap = src->capacity ? src->capacity : STACK_INITIAL_CAPACITY;  // handle capacity == 0

    // allocate memory
    copy->items = malloc(cap * sizeof(void *));
    if (!copy->items) {
        logFatal("Failed to allocate memory for stack copy items.");
    }
    // only copy items
    memcpy(copy->items, src->items, src->size * sizeof(void *));
    // set remainder to 0
    if (cap > src->size) {
        memset(copy->items + src->size, 0, (cap - src->size) * sizeof(void *));
    }
    copy->size = src->size;
    copy->capacity = src->capacity;
    
    return copy;
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

bool Stack_IsEmpty(const Stack *stack) {
    return stack->size == 0;
}

size_t Stack_Size(const Stack *stack) {
    return stack->size;
}

void Stack_Clear(Stack *stack) {
    if (!stack) {
        return;
    }
    stack->size = 0;
}
