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
#include "widgets/bottombar.h"
#include "widgets/app.h"


Widget *app;


void finish() {
    Widget_Destroy(app);
    Input_Deinit();
    Screen_ShowCursor();
    Screen_Deinit();
    Terminal_Deinit();
    printf("Goodbye!\n");
}

void onResize(int new_width, int new_height) {
    Widget_onParentResize(app, new_width, new_height);
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    atexit(finish);  // make sure original settings are restored

    Terminal_Init(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    Screen_Init(onResize);
    Input_Init();

    // initial screen draw
    Screen_Draw();
    Screen_HideCursor();

    app = App_Create(Screen_GetWidth(), Screen_GetHeight());
    
    Widget *label = Label_Create(app, "Hello, World!");
    label->width = 20;
    label->height = 1;

    Widget *bottombar = BottomBar_Create(app);
    (void)bottombar;

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


        Widget_Draw(app, &screen.canvas);
        Screen_Draw();

    }
}