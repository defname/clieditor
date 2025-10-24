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

Stack *Stack_Create();
void Stack_Destroy(Stack *stack);

Stack *Stack_Copy(const Stack *stack);

void Stack_Push(Stack *stack, void *item);
void *Stack_Pop(Stack *stack);
void *Stack_Peek(const Stack *stack);

bool Stack_Has(const Stack *stack, const void *item);

bool Stack_IsEmpty(const Stack *stack);
size_t Stack_Size(const Stack *stack);

void Stack_Clear(Stack *stack);


#endif