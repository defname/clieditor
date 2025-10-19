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
#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "io/terminal.h"
#include "common/utf8_helper.h"


Screen screen;

Screen screen;
static volatile sig_atomic_t resize_pending = 0;

static void on_resize(int sig) {  // WINCH signal handler
    (void)sig;
    resize_pending = 1;  // this is checked at the beginning of Screen_Draw()
}

static void handle_resize() {  // actually do the resize
    Screen_Clear();
    Terminal_Update(); // Update global terminal dimensions
    Canvas_Resize(&screen.canvas, terminal.cols, terminal.rows);
    if (screen.onResize) {
        screen.onResize(terminal.cols, terminal.rows);
    }
    Canvas_Clear(&screen.canvas);
}

void Screen_Init(ResizeCallback onResize) {
    Canvas_Init(&screen.canvas, terminal.cols, terminal.rows);
    screen.onResize = onResize;
    // Handler einrichten
    struct sigaction sa;
    sa.sa_handler = on_resize;
    sa.sa_flags = SA_RESTART; // Restart syscalls nach Signal
    sigemptyset(&sa.sa_mask);
    sigaction(SIGWINCH, &sa, NULL);
}

void Screen_Deinit() {
    Canvas_Deinit(&screen.canvas);
}

void Screen_HideCursor() {
    dprintf(terminal.fd_out, "\033[?25l");
}

void Screen_ShowCursor() {
    dprintf(terminal.fd_out, "\033[?25h");
}

//---------- DRAW -------------
// TODO: not all styles are implemented so far

void cursor_to(int col, int row) {
    dprintf(terminal.fd_out, "\033[%d;%dH", row+1, col+1);

}

void canvas_putchar(uint32_t cp) {
    char ch[4];
    size_t len = utf8_from_codepoint(cp, ch);
    write(terminal.fd_out, ch, len);
}

void underline(bool on) {
    if (on) {
        dprintf(terminal.fd_out, "\033[4m");
    }
    else {
        dprintf(terminal.fd_out, "\033[24m");
    }
}

void bold(bool on) {
    if (on) {
        dprintf(terminal.fd_out, "\033[1m");
    }
    else {
        dprintf(terminal.fd_out, "\033[22m");
    }
}

void update_style(const Style *current_style, const Style *new_style) {
    if (!current_style || current_style->fg != new_style->fg) {
        dprintf(terminal.fd_out, "\e[38;5;%dm", new_style->fg);
    }
    if (!current_style || current_style->bg != new_style->bg) {
        dprintf(terminal.fd_out, "\e[48;5;%dm", new_style->bg);
    }
    if (!current_style || current_style->attributes != new_style->attributes) {
        if (new_style->attributes & STYLE_BOLD) {
            bold(true);
        }
        else {
            bold(false);
        }
        if (new_style->attributes & STYLE_UNDERLINE) {
            underline(true);
        }
        else {
            underline(false);
        }
    }

}

void reset_style() {
    dprintf(terminal.fd_out, "\e[0m");
}

void Screen_Draw() {
    if (resize_pending) {
        handle_resize();
        resize_pending = 0;
    }

    bool skipped = false;
    int old_y = 0;
    Style current_style = { .fg = 12, .bg = 15, .attributes = STYLE_UNDERLINE };
    update_style(NULL, &current_style);
    cursor_to(0, 0);
    for (size_t i=0; i<screen.canvas.size; i++) {
        int x = i % screen.canvas.width;
        int y =i / screen.canvas.width;
        if (!screen.canvas.buffer[i].changed) {
            skipped = true;
            continue;
        }
        if (skipped || y != old_y) {
            cursor_to(x, y);
            skipped = false;
            old_y = y;
        }
        Cell *cell = &screen.canvas.buffer[i];
        update_style(&current_style, &cell->style);
        current_style = cell->style;
        
        canvas_putchar(screen.canvas.buffer[i].cp);
        screen.canvas.buffer[i].changed = false;
    }
    reset_style();
}

void Screen_Clear() {
    dprintf(terminal.fd_out, "\e[2J");
}


int Screen_GetWidth() {
    return screen.canvas.width;
}

int Screen_GetHeight() {
    return screen.canvas.height;
}
