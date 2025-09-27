#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "core/terminal.h"
#include "core/screen.h"
#include "core/utf8.h"
#include "core/input.h"

void finish() {
    Screen_Finish();Screen_Clear();
    Terminal_Finish();
}

int cursor_t = 0;

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    atexit(finish);  // make sure original settings are restored

    Terminal_Init(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    Screen_Init();

    // initial screen draw
    Screen_Clear();
    Screen_Draw();
    Screen_HideCursor();

    UTF8Char tmp;

    while (1) {
        EscapeSequence esc_seq = Input_Read(terminal.fd_in);

        switch (esc_seq) {
            case ESC_CURSOR_UP:
                Screen_MoveCursor(screen.cursor_col, screen.cursor_row - 1);
                break;
            case ESC_CURSOR_DOWN:
                Screen_MoveCursor(screen.cursor_col, screen.cursor_row + 1);
                break;
            case ESC_CURSOR_LEFT:
                Screen_MoveCursor(screen.cursor_col - 1, screen.cursor_row);
                break;
            case ESC_CURSOR_RIGHT:
                Screen_MoveCursor(screen.cursor_col + 1, screen.cursor_row);
                break;
            default:
                while ((tmp = Input_GetChar()).length != 0) {
                    Screen_PutChar(tmp);
                    Screen_Draw();
                }
                break;
        }
        // cursor blinking
        if (cursor_t + 100 < time(NULL)) {
            Cell under_curser = screen.buffer[screen.cursor_col + screen.cursor_row * terminal.cols];
            under_curser.style.attributes ^= STYLE_UNDERLINE;

            Screen_Draw();
        }
    }
}