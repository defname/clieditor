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
#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stdbool.h>

/**
 * @struct Buffer
 * @brief A generic circular buffer (ring buffer) implementation.
 *
 * This structure holds all the necessary information to manage a circular buffer
 * for any data type.
 */
typedef struct _Buffer {
    void *buffer;           /**< Pointer to the allocated memory block for the buffer's elements. */
    size_t capacity;        /**< The maximum number of elements the buffer can hold. */
    size_t element_size;    /**< The size of a single element in bytes. */
    size_t count;           /**< The current number of elements in the buffer. */
    size_t cursor;          /**< The index of the first (oldest) element in the buffer (the head). */
} Buffer;

/**
 * @brief Initializes a buffer.
 * @param buffer A pointer to the Buffer struct to initialize.
 * @param capacity The maximum number of elements the buffer can store.
 * @param element_size The size of each element in bytes (e.g., sizeof(int)).
 */
void Buffer_Init(Buffer *buffer, size_t capacity, size_t element_size);

/**
 * @brief Deinitializes a buffer, freeing its allocated memory.
 * @param buffer A pointer to the Buffer struct to deinitialize.
 */
void Buffer_Deinit(Buffer *buffer);

/**
 * @brief Adds an element to the end (tail) of the buffer.
 * If the buffer is full, the element is dropped.
 * @param buffer A pointer to the Buffer struct.
 * @param element A pointer to the element to be enqueued.
 */
void Buffer_Enqueue(Buffer *buffer, const void *element);

/**
 * @brief Removes an element from the front (head) of the buffer.
 * @param buffer A pointer to the Buffer struct.
 * @param out_element A pointer to a variable where the dequeued element will be copied.
 *                    If NULL, the element is removed without being copied.
 * @return true on success, false if the buffer was empty.
 */
bool Buffer_Dequeue(Buffer *buffer, void *out_element);

/**
 * @brief Checks if the buffer is empty.
 * @param buffer A pointer to the Buffer struct.
 * @return true if the buffer contains no elements, false otherwise.
 */
bool Buffer_IsEmpty(Buffer *buffer);

/** @brief Returns the current number of elements in the buffer. */
size_t Buffer_Size(Buffer *buffer);

/** @brief Returns the maximum capacity of the buffer. */
size_t Buffer_Capacity(Buffer *buffer);

/**
 * @brief Peeks at an element in the buffer without removing it.
 * @param buffer A pointer to the Buffer struct.
 * @param lookahead The 0-based index from the head of the buffer to peek at.
 * @param out_element A pointer to a variable where the peeked element will be copied.
 * @return true on success, false if the lookahead index is out of bounds.
 */
bool Buffer_Peek(Buffer *buffer, size_t lookahead, void *out_element);

#endif