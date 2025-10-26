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
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "io/terminal.h"
#include "io/screen.h"
#include "io/input.h"
#include "document/textbuffer.h"
#include "document/textio.h"
#include "io/timer.h"
#include "widgets/components/bottombar.h"
#include "widgets/app.h"
#include "widgets/components/editorview.h"
#include "widgets/primitives/notification.h"
#include "common/config.h"
#include "common/colors.h"
#include "common/callback.h"
#include "common/logging.h"
#include "common/iniparser.h"

#include "syntax/loader.h"

#include "widgets/primitives/frame.h"
#include "widgets/primitives/menu.h"


TextBuffer tb;
SyntaxHighlighting *highlighting;


static void print_help(const char *program_name) {
    fprintf(stderr, "Usage:  %s [options] <filename>\n\n", program_name);
    puts("  -h            Show this help");
    puts("  -s <format>   Specify a format for syntax highlighting.");
    puts("                The syntax definition file <format>.ini need to");
    puts("                present in data/syntax.");
    exit(0);
}

static void parse_arguments(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "hs:")) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                return;
            case 's':
                Config_SetSyntax(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                exit(1);
        }
    }

    // Verbleibende Argumente:
    for (int i = optind; i < argc; i++) {
        Config_SetFilename(argv[i]);
        return;
    }

    print_help(argv[0]);
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

static void onMenuClick(void *menu, void *entry) {
    (void)menu;
    (void)entry;
    if (strcmp(entry, "exit") == 0) {
        exit(0);
    }
    if (strcmp(entry, "save") == 0) {
        File *file = File_Open(Config_GetFilename(), FILE_ACCESS_WRITE);
        if (!file) {
            // Show error message
            // TODO
            Notification_Notify(app.notification, "Cannot open file for writing.", NOTIFICATION_ERROR);
            Widget_Hide(AS_WIDGET(menu));
            return;
        }
        TextBuffer_SaveToFile(&tb, file);
        File_Close(file);
        Widget_Hide(AS_WIDGET(menu));
        Notification_Notify(app.notification, "File saved", NOTIFICATION_SUCCESS);
    }
}

/************************************
 * Cleanup                          *
 ************************************/
static void finish() {  // called automatically (set with atexit())
    Timer_Deinit();
    App_Deinit();
    Config_Deinit();
    SyntaxHighlighting_Destroy(highlighting);
    TextBuffer_Deinit(&tb);
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

    Config_Init(argv[0]);
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

    TextBuffer_Init(&tb);

    SyntaxHighlightingLoaderError error;
    highlighting = SyntaxHighlighting_LoadFromFile(Config_GetSyntax(), &error);
    if (!highlighting) {
        switch (error.code) {
            case SYNTAX_LOADER_FILE_NOT_FOUND:
                logError("Syntax file not found.");
                break;
            case SYNTAX_LOADER_FILE_READ_ERROR:
                logError("Could not read syntax file.");
                break;
            case SYNTAX_LOADER_PARSE_ERROR:
                logError("Could not parse syntax file.\n%s", error.parsing_error.message);
                break;
            case SYNTAX_LOADER_DEFINITION_ERROR:
                logError("Syntax definition error.\n%s", error.def_error.message);
                break;
            default:
                logError("Could not load Syntaxdefinition (Code: %d)", error.code);
                break;
        }
        SyntaxHighlightingLoaderError_Deinit(&error);
        highlighting = NULL;
    }


    const char * fn = Config_GetFilename();
    bool failure_on_file_load = false;  // the failure message can only be shown after initializing the widget system
    if (strcmp(fn, "") != 0) {
        File *file;        
        file = File_Open(fn, FILE_ACCESS_READ);

        if (file) {
            TextBuffer_LoadFromFile(&tb, file);
            File_Close(file);
        }
        else {
            failure_on_file_load = true;
        }
    }

    // Load config
    File *config_file = File_OpenConfig(FILE_ACCESS_READ);
    if (config_file) {
        char *content = File_Read(config_file);
        if (content) {
            Config_LoadIni(content);
            free(content);
        }
        File_Close(config_file);
    }

    App_Init(Screen_GetWidth(), Screen_GetHeight());
 
    EditorView *editor = EditorView_Create(AS_WIDGET(&app), &tb);
    Widget_Focus(AS_WIDGET(editor));
    editor->editor->sh_binding.sh = highlighting;
    (void)editor;
    BottomBar *bottombar = BottomBar_Create(AS_WIDGET(&app));
    (void)bottombar;
    
    
    MenuEntry entries[] = {
        {
            .text = "Save",
            .shortcut = 's',
            .callback = Callback_New(onMenuClick, "save")
        },
        {
            .text = "Exit",
            .shortcut = 'q',
            .callback = Callback_New(onMenuClick, "exit")
        },
    };
    Menu *menu = Menu_Create(entries, sizeof(entries) / sizeof(MenuEntry), "CliEditor", (Callback){NULL, NULL});
    Widget_Hide(AS_WIDGET(menu));

    Widget_SortTreeByZIndex(AS_WIDGET(&app));
    App_onParentResize(Screen_GetWidth(), Screen_GetHeight());  // trigger on_resize on all widgets


    // failure message from file load 
    if (failure_on_file_load) {
        Notification_Notify(app.notification, "Cannot open file for reading.", NOTIFICATION_WARNING);
    }


    /************************************
     * Main loop                        *
     ************************************/
    while (1) {
        Timer_Update();
        
        InputEvent input = Input_Read();
        
        if (InputEvent_IsValid(&input) && !App_HandleInput(input)) {
            if (input.key == KEY_ESC) {
                Widget_FocusAndReturn(AS_WIDGET(menu), AS_WIDGET(&app));
            }
        }

        App_Update();
        Config_Loaded();

        App_Draw(&screen.canvas);
        Screen_Draw();

    }
}