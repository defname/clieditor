#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "core/terminal.h"
#include "core/screen.h"
#include "core/utf8string.h"
#include "core/input.h"
#include "widgets/label.h"

void finish() {
    Input_Deinit();
    Screen_Deinit();
    Terminal_Deinit();
    printf("Goodbye!\n");
}

int cursor_t = 0;

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    atexit(finish);  // make sure original settings are restored

    Terminal_Init(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    Screen_Init();
    Input_Init();

    // initial screen draw
    Screen_Draw();
    Screen_HideCursor();


    Widget *label = Label_Create(NULL, "Hello, World!");

    while (1) {
        EscapeSequence esc_seq = Input_Read(terminal.fd_in);
        (void)esc_seq;
        if (esc_seq == ESC_CURSOR_DOWN) {
            label->y++;
        }
        else if (esc_seq == ESC_CURSOR_UP) {
            label->y--;
        }
        else if (esc_seq == ESC_CURSOR_LEFT) {
            label->x--;
        }
        else if (esc_seq == ESC_CURSOR_RIGHT) {
            label->x++;
        }
        else if (esc_seq == ESC_ESCAPE) {
            return 0;
        }

        UTF8Char tmp;
        while ((tmp = Input_GetChar()).length != 0) {
            UTF8String_AddChar(&(((LabelData*)(label->data))->text), tmp);
        }


        label->ops->draw(label, &screen.canvas);
        Screen_Draw();

        /*
        if (esc_seq != ESC_NONE) {
            // escape sequence
        }
        while ((tmp = Input_GetChar()).length != 0) {
            if (tmp.length == 1 && tmp.bytes[0] == '\e') {
                return 0;
            }
            Screen_PutChar(tmp);
            Screen_Draw();
        }
        
        // cursor blinking
        if (cursor_t + 1 < time(NULL)) {
            cursor_t = time(NULL);
            Cell *under_curser = &screen.buffer[screen.cursor_col + screen.cursor_row * terminal.cols];
            under_curser->style.attributes ^= STYLE_UNDERLINE;
            under_curser->changed = true;

            Screen_Draw();
        }
        */
    }
}