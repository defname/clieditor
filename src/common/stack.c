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

static void resize_capacity(Stack *stack, size_t new_capacity) {
    if (!stack || stack->capacity >= new_capacity) {
        return;
    }
    stack->items = realloc(stack->items, new_capacity * sizeof(void *));
    if (!stack->items) {
        logFatal("Failed to reallocate stack items.");
    }
    stack->capacity = new_capacity;
}

static void increase_capacity(Stack *stack) {
    if (!stack) {
        return;
    }
    size_t new_cap = stack->capacity == 0 ? STACK_INITIAL_CAPACITY : stack->capacity * STACK_GROW_FACTOR;
    resize_capacity(stack, new_cap);
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
    Stack *copy = Stack_Create();
    Stack_CopyTo(copy, src);
    return copy;
}

void Stack_CopyTo(Stack *dst, const Stack *src) {
    if (!dst || !src) {
        return;
    }
    if (src->size == 0) {
        // ealry exit if src is empty
        Stack_Clear(dst);
        return;
    }
    if (src->size > dst->capacity) {
        // resize dst to a multiple of STACK_INITIAL_SIZE
        size_t new_cap = ((src->size % STACK_INITIAL_CAPACITY) + 1) * STACK_INITIAL_CAPACITY;
        resize_capacity(dst, new_cap);
    }
    // copy items
    memcpy(dst->items, src->items, src->size * sizeof(void *));
    // set remainder to 0
    if (dst->capacity > src->size) {
        memset(dst->items + src->size, 0, (dst->capacity - src->size) * sizeof(void *));
    }
    dst->size = src->size;
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

void Stack_Clear(Stack *stack) {
    if (!stack) {
        return;
    }
    stack->size = 0;
}
