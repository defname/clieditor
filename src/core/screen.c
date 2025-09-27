#include "screen.h"

#include "stdio.h"
#include "terminal.h"

Screen screen;

void Screen_Init() {
    Canvas_Init(&screen.canvas, terminal.cols, terminal.rows);
}

void Screen_Deinit() {
    Canvas_Deinit(&screen.canvas);
}

void Screeen_onResize() {
    Canvas_Resize(&screen.canvas, terminal.cols, terminal.rows);
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
        cursor_to(i % terminal.cols, i / terminal.cols);
        underline(screen.canvas.buffer[i].style.attributes & STYLE_UNDERLINE);
        
        canvas_putchar(screen.canvas.buffer[i].ch);
        screen.canvas.buffer[i].changed = false;
    }
}