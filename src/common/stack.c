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
