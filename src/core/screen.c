#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "terminal.h"

Screen screen;
bool resize_pending = false;

static void on_resize(int sig) {  // WINCH signal handler
    (void)sig;
    resize_pending = true;  // this is checked at the beginning of Screen_Draw()
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

void canvas_putchar(UTF8Char c) {
    UTF8_PutChar(terminal.fd_out, c);
}

void underline(bool on) {
    if (on) {
        dprintf(terminal.fd_out, "\033[4m");
    }
    else {
        dprintf(terminal.fd_out, "\033[24m");
    }
}

void bold() {
    dprintf(terminal.fd_out, "\033[1m");
}

void update_style(const Style *current_style, const Style *new_style) {
    if (!current_style || current_style->fg != new_style->fg) {
        dprintf(terminal.fd_out, "\e[38;5;%dm", new_style->fg);
    }
    if (!current_style || current_style->bg != new_style->bg) {
        dprintf(terminal.fd_out, "\e[48;5;%dm", new_style->bg);
    }

}

void reset_style() {
    dprintf(terminal.fd_out, "\e[0m");
}

void Screen_Draw() {
    if (resize_pending) {
        handle_resize();
        resize_pending = false;
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
        
        canvas_putchar(screen.canvas.buffer[i].ch);
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
