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
#include "buffer.h"

#include <string.h>
#include <stdlib.h>
#include "common/logging.h"



void Buffer_Init(Buffer *buffer, size_t capacity, size_t element_size) {
    buffer->buffer = malloc(capacity * element_size);
    if (buffer->buffer == NULL) {
        logFatal("Cannot allocate memory for buffer.");
    }
    buffer->capacity = capacity;
    buffer->element_size = element_size;
    buffer->count = 0;
    buffer->cursor = 0;
}

void Buffer_Deinit(Buffer *buffer) {
    if (buffer->buffer) {
        free(buffer->buffer);
    }
    buffer->buffer = NULL;
}

void Buffer_Enqueue(Buffer *buffer, const void *element) {
    if (buffer->count >= buffer->capacity) {
        logError("Buffer overflow. Element dropped.");
        return;
    }
    size_t new_pos = (buffer->cursor + buffer->count) % buffer->capacity;
    buffer->count++;
    memcpy(buffer->buffer + new_pos * buffer->element_size, element, buffer->element_size);
}

bool Buffer_Dequeue(Buffer *buffer, void *out_element) {
    if (buffer->count == 0) {
        logError("Buffer underflow on Dequeue.");
        return false;
    }
    // If an output buffer is provided, copy the data.
    if (out_element != NULL) {
        void *source = buffer->buffer + buffer->cursor * buffer->element_size;
        memcpy(out_element, source, buffer->element_size);
    }
    // Advance the cursor regardless.
    buffer->cursor = (buffer->cursor + 1) % buffer->capacity;
    buffer->count--;
    return true;
}

bool Buffer_IsEmpty(Buffer *buffer) {
    return buffer->count == 0;
}

size_t Buffer_Size(Buffer *buffer) {
    return buffer->count;
}

size_t Buffer_Capacity(Buffer *buffer) {
    return buffer->capacity;
}

bool Buffer_Peek(Buffer *buffer, size_t lookahead, void *out_element) {
    if (lookahead >= buffer->count) {
        logError("Buffer underflow on Peek (lookahead: %zu, count: %zu).", lookahead, buffer->count);
        return false;
    }
    if (out_element == NULL) {
        return false; // Nothing to do if no output buffer is provided.
    }
    size_t peek_pos = (buffer->cursor + lookahead) % buffer->capacity;
    memcpy(out_element, buffer->buffer + peek_pos * buffer->element_size, buffer->element_size);
    return true;
}