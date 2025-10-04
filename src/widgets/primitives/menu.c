#include "menu.h"
#include <string.h>
#include "widgets/app.h"
#include "io/screen.h"
#include "common/colors.h"
#include "common/logging.h"
#include "common/utf8string.h"


static bool on_input(Widget *self, EscapeSequence key, UTF8Char ch) {
    (void)key;
    (void)ch;
    Menu *menu = AS_MENU(self);
    if (key == ESC_ESCAPE) {
        Callback_Call(&menu->on_close, self);
        Widget_Hide(self);
    }
    else if (key == ESC_CURSOR_UP) {
        if (menu->selected_entry > 0) {
            menu->selected_entry--;
        }
    }
    else if (key == ESC_CURSOR_DOWN) {
        if (menu->selected_entry < menu->entry_count - 1) {
            menu->selected_entry++;
        }
    }
    else if (UTF8_IsASCII(ch)) {
        char c = UTF8_AsASCII(ch);
        if (c == KEY_ENTER || c == ' ') {
            MenuEntry *entry = &menu->entries[menu->selected_entry];
            Callback_Call(&entry->callback, self);
        }
    }

    return true;
}

static void on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    (void)new_parent_width;
    (void)new_parent_height;
    Menu *menu = AS_MENU(self);
    int max_len = 0;
    for (size_t i=0; i<menu->entry_count; i++) {
        int len = strlen(menu->entries[i].text);
        if (len > max_len) {
            max_len = len;
        }
    }
    self->width = max_len + 2 * MENU_BORDER_X;
    self->height = AS_MENU(self)->entry_count + 2 * MENU_BORDER_Y;
    self->x = (Screen_GetWidth() - self->width) / 2;
    self->y = (Screen_GetHeight() - self->height) / 2;
}

static void draw(const Widget *self, Canvas *canvas) {
    (void)self;
    (void)canvas;
    Menu *menu = AS_MENU(self);
    UTF8String text;
    UTF8String_Init(&text);
    for (size_t i=0; i<menu->entry_count; i++) {
        Canvas_MoveCursor(canvas, MENU_BORDER_X, i + MENU_BORDER_Y);
        const char *entry_text = menu->entries[i].text;
        UTF8String_FromStr(&text, entry_text, strlen(entry_text));
        if (i == menu->selected_entry) {
            canvas->current_style.attributes |= STYLE_UNDERLINE;
        }
        Canvas_Write(canvas, &text);
        if (i == menu->selected_entry) {
            canvas->current_style.attributes &= ~STYLE_UNDERLINE;
        }
    }
    UTF8String_Deinit(&text);
}

WidgetOps mainmenu_ops = {
    .draw = draw,
    .on_input = on_input,
    .on_resize = on_resize,
};

void Menu_Init(Menu *menu, MenuEntry *entries, size_t entry_count, Callback on_close) {
    Widget_Init(&menu->base, AS_WIDGET(&app), &mainmenu_ops);
    menu->entries = entries;
    menu->entry_count = entry_count;
    menu->selected_entry = 0;
    menu->on_close = on_close;

    AS_WIDGET(menu)->style.fg = Color_GetCodeById(COLOR_BG);
    AS_WIDGET(menu)->style.bg = Color_GetCodeById(COLOR_PRIMARY_BG);
    AS_WIDGET(menu)->style.attributes = STYLE_BOLD;
}

Menu *Menu_Create(MenuEntry *entries, size_t entry_count, Callback on_close) {
    Menu *self = malloc(sizeof(Menu));
    if (!self) {
        logFatal("Cannot allocate memory for MainMenu.");
    }
    Menu_Init(self, entries, entry_count, on_close);
    return self;
}
