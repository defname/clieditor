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
#include "document/textio.h"
#include "io/timer.h"
#include "widgets/components/bottombar.h"
#include "widgets/app.h"
#include "widgets/components/editor.h"
#include "common/config.h"
#include "common/colors.h"
#include "common/callback.h"

#include "widgets/primitives/frame.h"
#include "widgets/primitives/menu.h"

TextBuffer tb;

static void parse_arguments(int argc, char *argv[]) {
    // For now, we only handle a single filename argument.
    if (argc >= 2) {
        Config_SetFilename(argv[1]);
    } else {
        // No file provided, could set a default or leave it empty.
        Config_SetFilename(NULL);
    }
}

static void load_environment() {
    const char *colorterm = getenv("COLORTERM");
    if (colorterm) {
        Color_SetMode(COLOR_MODE_256);
    } else {
        Color_SetMode(COLOR_MODE_16);
    }
}

static void onResize(int new_width, int new_height) {
    App_onParentResize(new_width, new_height);
}

static void onMenuClose(void *menu, void *new_focus) {
    Widget_Hide(AS_WIDGET(menu));
    (void)new_focus;
    //Widget_Focus(AS_WIDGET(new_focus));
}

static void onMenuClick(void *menu, void *entry) {
    (void)menu;
    (void)entry;
    if (strcmp(entry, "exit") == 0) {
        exit(0);
    }
}

/************************************
 * Cleanup                          *
 ************************************/
static void finish() {  // called automatically (set with atexit())
    Timer_Deinit();
    App_Deinit();
    Config_Deinit();
    TB_Deinit(&tb);
    Input_Deinit();
    Screen_Deinit();
    Terminal_Deinit();
    Screen_ShowCursor();
    printf("Goodbye!\n");
}

int main(int argc, char *argv[]) {

    /************************************
     * Initialization                   *
     ************************************/
    srand(time(NULL));

    Config_Init();
    parse_arguments(argc, argv);
    load_environment();

    atexit(finish);  // make sure original settings are restored
    
    Terminal_Init(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    Screen_Init(onResize);
    Input_Init();
    Timer_Init();

    // initial screen draw
    Screen_Draw();
    Screen_HideCursor();

    TB_Init(&tb);

    const char * fn = Config_GetFilename();
    if (strcmp(fn, "") != 0) {
        File *file;        
        file = File_Open(fn, FILE_ACCESS_READ);

        if (file) {
            TB_LoadFromFile(&tb, file);
            File_Close(file);
        }
    }

    App_Init(Screen_GetWidth(), Screen_GetHeight());
 
    Editor *editor = Editor_Create(AS_WIDGET(&app), &tb);
    Widget_Focus(AS_WIDGET(editor));
    (void)editor;
    BottomBar *bottombar = BottomBar_Create(AS_WIDGET(&app));
    (void)bottombar;
    

/*
    Frame *frame = Frame_Create(AS_WIDGET(&app));
    frame->base.x = 10;
    frame->base.y = 10;
    frame->base.width = 20;
    frame->base.height = 5;

    Widget_SetZIndex((Widget*)frame, 10);
    App_SetFocus((Widget*)frame);
*/
    MenuEntry entries[] = {
        { .text = "Save", Callback_New(onMenuClick, "save") },
        { .text = "Exit", Callback_New(onMenuClick, "exit") },
    };
    Menu *menu = Menu_Create(entries, sizeof(entries) / sizeof(MenuEntry), Callback_New(onMenuClose, editor));
    Widget_Hide(AS_WIDGET(menu));

    App_onParentResize(Screen_GetWidth(), Screen_GetHeight());

    /************************************
     * Main loop                        *
     ************************************/
    while (1) {
        Timer_Update();
        EscapeSequence esc_seq = Input_Read();
        
        if (!App_HandleInput(esc_seq, utf8_invalid)) {
            if (esc_seq == ESC_ESCAPE) {
                Widget_FocusAndReturn(AS_WIDGET(menu), AS_WIDGET(&app));
            }
        }
        UTF8Char ch;
        while ((ch = Input_GetChar()).length != 0) {
            App_HandleInput(ESC_NONE, ch);
        }

        App_Draw(&screen.canvas);
        Screen_Draw();

    }
}