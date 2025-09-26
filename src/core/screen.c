#include "screen.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "terminal.h"
#include "utf8.h"
#include "../utils/logging.h"


Screen screen;

void reallocate_buffers() {
    size_t new_min_capacity = terminal.cols * terminal.rows;
    screen.size = new_min_capacity;
    if (new_min_capacity <= screen.capacity) {
        return;
    }
    screen.buffer = realloc(screen.buffer, sizeof(Cell) * new_min_capacity);
    if (!screen.buffer) {
        logFatal("Cannot allocate memory for screen buffer.");
    }
    screen.capacity = new_min_capacity;
}

void Screen_Init() {
    screen.buffer = NULL;
    screen.capacity = 0;
    screen.size = 0;
    reallocate_buffers();
}

void Screen_Finish() {
    if (screen.buffer != NULL) {
        free(screen.buffer);
    }
    screen.buffer = NULL;
    screen.capacity = 0;
    screen.size = 0;
}

void Screen_Update() {
    reallocate_buffers();
}

void cursor_increment() {
    screen.cursor_col++;
    if (screen.cursor_col >= terminal.cols) {
        screen.cursor_col = 0;
        screen.cursor_row++;
    }
    if (screen.cursor_row >= terminal.rows) {
        screen.cursor_row = 0;
    }
}

void Screen_Clear() {
    const UTF8Char blank = { .bytes = {' '}, .length = 1 };
    const Cell blank_cell = { .ch = blank, .style = { .fg = 0, .bg = 0, .attributes = STYLE_NONE }, .changed = true };

    for (size_t i = 0; i < screen.size; i++) {
        screen.buffer[i] = blank_cell;
    }
}

void Screen_MoveCursor(int col, int row) {
    screen.cursor_col = col;
    screen.cursor_row = row;
}

void Screen_HideCursor() {
    dprintf(terminal.fd_out, "\033[?25l");
}

void Screen_ShowCursor() {
    dprintf(terminal.fd_out, "\033[?25h");
}

void Screen_PutChar(UTF8Char c) {
    if (screen.cursor_col >= terminal.cols || screen.cursor_row >= terminal.rows) {
        logDebug("Cursor out of bounds.");
        return;
    }
    int idx = screen.cursor_col + screen.cursor_row * terminal.cols;
    if (!UTF8_Equal(screen.buffer[idx].ch, c) || memcmp(&screen.buffer[idx].style, &screen.current_style, sizeof(Style)) != 0) {
        screen.buffer[idx].ch = c;
        screen.buffer[idx].style = screen.current_style;
        screen.buffer[idx].changed = true;
    }
    cursor_increment();
}

void Screen_Write(const UTF8Char *s, size_t n) {
    if (n > screen.size) {
        n = screen.size;
    }
    for (size_t i=0; i<n; i++) {
        if (UTF8_EqualToChar(s[i], '\0')) {
            break;
        }
        Screen_PutChar(s[i]);
    }
}

//---------- DRAW -------------

void cursor_to(int col, int row) {
    dprintf(terminal.fd_out, "\033[%d;%dH", row+1, col+1);

}

void screen_putchar(UTF8Char c) {
    UTF8_PutChar(terminal.fd_out, c);
}

void Screen_Draw() {
    for (size_t i=0; i<screen.size; i++) {
        if (!screen.buffer[i].changed) {
            continue;
        }
        cursor_to(i % terminal.cols, i / terminal.cols);
        screen_putchar(screen.buffer[i].ch);
        screen.buffer[i].changed = false;
    }
}