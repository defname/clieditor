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
#include "menu.h"
#include <string.h>
#include "widgets/app.h"
#include "io/screen.h"
#include "common/colors.h"
#include "common/logging.h"


static void destroy(Widget *self) {
    Menu *menu = AS_MENU(self);
    UTF8String_Deinit(&menu->title);
}

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
    // check for shortcuts
    if (ch.length > 0) {
        for (size_t i=0; i<menu->entry_count; i++) {
            if (UTF8_Equal(ch, menu->entries[i].shortcut)) {
                MenuEntry *entry = &menu->entries[i];
                Callback_Call(&entry->callback, self);
            }
        }
    }

    return true;
}

static void on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    (void)new_parent_width;
    (void)new_parent_height;
    Menu *menu = AS_MENU(self);
    int max_len = menu->title.length + MENU_BORDER_X * 2;
    for (size_t i=0; i<menu->entry_count; i++) {
        int len = strlen(menu->entries[i].text) + 5;  // distance to print shortcut
        if (len > max_len) {
            max_len = len;
        }
    }
    self->width = max_len + 2 * MENU_BORDER_X;
    self->height = AS_MENU(self)->entry_count + 2 * MENU_BORDER_Y;
    if (menu->title.length > 0) {
        self->height += 2;
    }
    self->x = (Screen_GetWidth() - self->width) / 2;
    self->y = (Screen_GetHeight() - self->height) / 2;
}

static void draw(const Widget *self, Canvas *canvas) {
    (void)self;
    (void)canvas;
    Menu *menu = AS_MENU(self);

    int y_offset = 0;
    if (menu->title.length > 0) {
        int x = (self->width - menu->title.length) / 2;
        Canvas_MoveCursor(canvas, x, MENU_BORDER_Y);
        Canvas_Write(canvas, &menu->title);
        y_offset += 2;
    }


    UTF8String text;
    UTF8String_Init(&text);
    for (size_t i=0; i<menu->entry_count; i++) {
        int y = i + MENU_BORDER_Y + y_offset;
        Canvas_MoveCursor(canvas, MENU_BORDER_X, y);
        const char *entry_text = menu->entries[i].text;
        int text_len = strlen(entry_text);
        UTF8String_FromStr(&text, entry_text, text_len);
        if (i == menu->selected_entry) {
            canvas->current_style.attributes |= STYLE_UNDERLINE;
        }
        Canvas_Write(canvas, &text);
        
        // draw shortcut hint
        if (menu->entries[i].shortcut.length > 0) {
            int right_x = self->width - MENU_BORDER_X - 1;
            Canvas_MoveCursor(canvas, right_x, y);
            Canvas_PutChar(canvas, menu->entries[i].shortcut);
        }

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
    .destroy = destroy,
};

void Menu_Init(Menu *menu, MenuEntry *entries, size_t entry_count, const char *title, Callback on_close) {
    Widget_Init(&menu->base, AS_WIDGET(&app), &mainmenu_ops);
    menu->entries = entries;
    menu->entry_count = entry_count;
    menu->selected_entry = 0;
    menu->on_close = on_close;
    UTF8String_Init(&menu->title);

    if (title) {
        UTF8String_FromStr(&menu->title, title, strlen(title));
    }


    AS_WIDGET(menu)->style.fg = Color_GetCodeById(COLOR_BG);
    AS_WIDGET(menu)->style.bg = Color_GetCodeById(COLOR_PRIMARY_BG);
    AS_WIDGET(menu)->style.attributes = STYLE_BOLD;
}

Menu *Menu_Create(MenuEntry *entries, size_t entry_count, const char *title, Callback on_close) {
    Menu *self = malloc(sizeof(Menu));
    if (!self) {
        logFatal("Cannot allocate memory for MainMenu.");
    }
    Menu_Init(self, entries, entry_count, title, on_close);
    return self;
}
