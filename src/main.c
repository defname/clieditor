#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "io/terminal.h"
#include "io/screen.h"
#include "common/utf8string.h"
#include "io/input.h"
#include "document/textbuffer.h"
#include "io/timer.h"
#include "widgets/label.h"
#include "widgets/bottombar.h"
#include "widgets/app.h"
#include "widgets/editor.h"

Widget *app;
TextBuffer tb;

void finish() {
    Widget_Destroy(app);
    TB_Deinit(&tb);
    Input_Deinit();
    Screen_Deinit();
    Terminal_Deinit();
    Screen_ShowCursor();
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
    Timer_Init();

    // initial screen draw
    Screen_Draw();
    Screen_HideCursor();

    TB_Init(&tb);

    app = App_Create(Screen_GetWidth(), Screen_GetHeight());
    
    Widget *editor = Editor_Create(app, &tb);
    (void)editor;
    Widget *bottombar = BottomBar_Create(app);
    (void)bottombar;

    Widget_onParentResize(app, Screen_GetWidth(), Screen_GetHeight());

    while (1) {
        Timer_Update();
        EscapeSequence esc_seq = Input_Read();
        if (esc_seq == ESC_ESCAPE) {
            return 0;
        }

        Widget_HandleInput(app, esc_seq, utf8_invalid);


        UTF8Char ch;
        while ((ch = Input_GetChar()).length != 0) {
            Widget_HandleInput(app, ESC_NONE, ch);
        }


        Widget_Draw(app, &screen.canvas);
        Screen_Draw();

    }
}