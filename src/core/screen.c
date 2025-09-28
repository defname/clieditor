#include "screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "terminal.h"

Screen screen;


static void on_resize(int sig) {
    (void)sig;
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl TIOCGWINSZ");
        return;
    }
    Terminal_Update(); // Update global terminal dimensions
    Canvas_Resize(&screen.canvas, w.ws_col, w.ws_row);
    if (screen.onResize) {
        screen.onResize(w.ws_col, w.ws_row);
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

void Screen_Draw() {
    for (size_t i=0; i<screen.canvas.size; i++) {
        if (!screen.canvas.buffer[i].changed) {
            continue;
        }
        cursor_to(i % screen.canvas.width, i / screen.canvas.width);
        underline(screen.canvas.buffer[i].style.attributes & STYLE_UNDERLINE);
        
        canvas_putchar(screen.canvas.buffer[i].ch);
        screen.canvas.buffer[i].changed = false;
    }
    underline(false);
}

int Screen_GetWidth() {
    return screen.canvas.width;
}

int Screen_GetHeight() {
    return screen.canvas.height;
}

