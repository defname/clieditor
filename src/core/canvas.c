#include "canvas.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "terminal.h"
#include "utf8.h"
#include "utils/logging.h"


void reallocate_buffers(Canvas *canvas) {
    size_t new_min_capacity = canvas->width * canvas->height;
    canvas->size = new_min_capacity;
    if (new_min_capacity <= canvas->capacity) {
        return;
    }
    size_t old_capacity = canvas->capacity;
    Cell *buffer = realloc(canvas->buffer, sizeof(Cell) * new_min_capacity);
    if (!buffer) {
        logFatal("Cannot allocate memory for canvas buffer.");
    }
    
    for (size_t i = old_capacity; i < new_min_capacity; ++i) {
        Cell_Init(&buffer[i]);
    }
    canvas->buffer = buffer;
    canvas->capacity = new_min_capacity;
}

void Canvas_Init(Canvas *canvas, int width, int height) {
    canvas->buffer = NULL;
    canvas->capacity = 0;
    canvas->size = 0;
    canvas->cursor_x = 0;
    canvas->cursor_y = 0;
    canvas->current_style = (Style) { .fg = 1, .bg = 0, .attributes = STYLE_NONE };
    Canvas_Resize(canvas, width, height);
}

void Canvas_Deinit(Canvas *canvas) {
    if (canvas->buffer != NULL) {
        free(canvas->buffer);
    }
    canvas->buffer = NULL;
    canvas->capacity = 0;
    canvas->size = 0;
}

void Canvas_Resize(Canvas *canvas, int width, int height) {
    canvas->width = width;
    canvas->height = height;
    reallocate_buffers(canvas);
}

void update_cell(Canvas *canvas, int col, int row, Cell *origin) {
    if (col >= canvas->width || row >= canvas->height || col < 0 || row < 0) {
        return;
    }
    Cell *cell = &canvas->buffer[row * canvas->width + col];
    if (Cell_Equal(cell, origin)) {
        cell->changed = false;
        return;
    }
    cell->ch = origin->ch;
    cell->style = origin->style;
    cell->changed = true;
}

void Canvas_ClipTo(Canvas *canvas, Canvas *target, int x, int y) {
    for (int col=0; col < canvas->width; col++) {
        for (int row=0; row < canvas->height; row++) {
            size_t idx = row * canvas->width + col;
            if (idx >= canvas->size) {
                logDebug("canvas index out of bounds.");
                continue;
            }
            Cell *origin = &canvas->buffer[idx];
            update_cell(target, x+col, y+row, origin);
        }
    }
}


void cursor_increment(Canvas *canvas) {
    canvas->cursor_x++;
    if (canvas->cursor_x >= canvas->width) {
        canvas->cursor_x = 0;
        canvas->cursor_y++;
    }
    if (canvas->cursor_y >= canvas->height) {
        canvas->cursor_y = 0;
    }
}

void Canvas_Clear(Canvas *canvas) {
    const UTF8Char blank = { .bytes = {' '}, .length = 1 };
    const Cell blank_cell = { .ch = blank, .style = { .fg = 0, .bg = 0, .attributes = STYLE_NONE }, .changed = true };

    for (size_t i = 0; i < canvas->size; i++) {
        canvas->buffer[i] = blank_cell;
    }
}

void Canvas_MoveCursor(Canvas *canvas, int col, int row) {
    canvas->cursor_x = col;
    canvas->cursor_y = row;
}

void Canvas_PutChar(Canvas *canvas, UTF8Char c) {
    if (canvas->cursor_x >= canvas->width
        || canvas->cursor_y >= canvas->height
        || canvas->cursor_x < 0
        || canvas->cursor_y < 0) {
        //logDebug("Cursor out of bounds.");
        canvas->cursor_x++;  // so Canvas_Write works if starting out of screen
        return;
    }
    // Use the canvas's width for index calculation, not the terminal's.
    int idx = canvas->cursor_x + canvas->cursor_y * canvas->width;
    if (!UTF8_Equal(canvas->buffer[idx].ch, c) || memcmp(&canvas->buffer[idx].style, &canvas->current_style, sizeof(Style)) != 0) {
        canvas->buffer[idx].ch = c;
        canvas->buffer[idx].style = canvas->current_style;
        canvas->buffer[idx].changed = true;
    }
    canvas->cursor_x++; // Move cursor to the right. Wrapping is handled by the caller or by subsequent calls.
}

void Canvas_Write(Canvas *canvas, const UTF8Char *s, size_t n) {
    if (n > canvas->size) {
        n = canvas->size;
    }
    for (size_t i=0; i<n; i++) {
        if (UTF8_EqualToChar(s[i], '\0')) {
            break;
        }
        Canvas_PutChar(canvas, s[i]);
    }
}
