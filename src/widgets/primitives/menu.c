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
#include "common/config.h"
#include "common/utf8_helper.h"


static void destroy(Widget *self) {
    Menu *menu = AS_MENU(self);
    String_Deinit(&menu->title);
}

static bool on_input(Widget *self, InputEvent input) {

    Menu *menu = AS_MENU(self);
    if (input.key == KEY_ESC) {
        Callback_Call(&menu->on_close, self);
        Widget_Hide(self);
        return true;
    }
    else if (input.key == KEY_UP) {
        if (menu->selected_entry > 0) {
            menu->selected_entry--;
            return true;
        }
    }
    else if (input.key == KEY_DOWN) {
        if (menu->selected_entry < menu->entry_count - 1) {
            menu->selected_entry++;
            return true;
        }
    }
    else if (input.key == KEY_ENTER || input.ch == ' ') {
        MenuEntry *entry = &menu->entries[menu->selected_entry];
        Callback_Call(&entry->callback, self);
        return true;
    }
    // check for shortcuts
    if (input.ch != 0x00 && input.ch != INVALID_CODEPOINT) {
        for (size_t i = 0; i < menu->entry_count; i++) {
            uint32_t shortcut = menu->entries[i].shortcut;
            if (shortcut == 0x00 || shortcut == INVALID_CODEPOINT) {
                continue;
            }
            if (input.ch == shortcut) {
                MenuEntry *entry = &menu->entries[i];
                Callback_Call(&entry->callback, self);
                return true;
            }
        }
    }

    return false;
}

static void on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    (void)new_parent_width;
    (void)new_parent_height;
    Menu *menu = AS_MENU(self);
    int max_len = String_Length(&menu->title) + MENU_BORDER_X * 2;
    for (size_t i=0; i<menu->entry_count; i++) {
        int len = strlen(menu->entries[i].text) + 5;  // distance to print shortcut
        if (len > max_len) {
            max_len = len;
        }
    }
    self->width = max_len + 2 * MENU_BORDER_X;
    self->height = AS_MENU(self)->entry_count + 2 * MENU_BORDER_Y;
    if (String_Length(&menu->title) > 0) {
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
    if (String_Length(&menu->title) > 0) {
        int x = (self->width - String_Length(&menu->title)) / 2;
        Canvas_MoveCursor(canvas, x, MENU_BORDER_Y);
        Canvas_Write(canvas, &menu->title);
        y_offset += 2;
    }

    Style orig_style = canvas->current_style;

    for (size_t i=0; i<menu->entry_count; i++) {
        int y = i + MENU_BORDER_Y + y_offset;
        int display_width = self->width - MENU_BORDER_X * 2;
        uint32_t shortcut = menu->entries[i].shortcut ? menu->entries[i].shortcut : ' ';
        char shortcut_ch[4];
        size_t shortcut_byte_len = utf8_from_codepoint(shortcut, shortcut_ch); 
        String text = String_Format("%*s%*s", display_width - 1, menu->entries[i].text, shortcut_byte_len, shortcut_ch);

        Canvas_MoveCursor(canvas, MENU_BORDER_X, y);
        if (i == menu->selected_entry) {
            canvas->current_style = menu->style_selected;
        }
        Canvas_Write(canvas, &text);

        if (i == menu->selected_entry) {
            canvas->current_style = orig_style;
        }
        String_Deinit(&text);
    }
    
}

static void on_config_change(Widget *w) {
    Menu *self = AS_MENU(w);
    Table *conf = Config_GetModuleConfig("menu");

    w->style.fg = Config_GetColor(conf, "text", self->base.style.fg);
    w->style.bg = Config_GetColor(conf, "bg", self->base.style.bg);
    self->style_selected.fg = Config_GetColor(conf, "selected.text", self->style_selected.fg);
    self->style_selected.attributes = Config_GetNumber(conf, "selected.attributes", self->style_selected.attributes);
}   

WidgetOps mainmenu_ops = {
    .draw = draw,
    .on_input = on_input,
    .on_resize = on_resize,
    .on_config_changed = on_config_change,
    .destroy = destroy,
};

void Menu_Init(Menu *menu, MenuEntry *entries, size_t entry_count, const char *title, Callback on_close) {
    Widget_Init(&menu->base, AS_WIDGET(&app), &mainmenu_ops);
    menu->entries = entries;
    menu->entry_count = entry_count;
    menu->selected_entry = 0;
    menu->on_close = on_close;

    if (title) {
        menu->title = String_FromCStr(title, strlen(title));
    }
    else {
        menu->title = String_Empty();
    }


    AS_WIDGET(menu)->style.fg = Color_GetCodeById(COLOR_BG);
    AS_WIDGET(menu)->style.bg = Color_GetCodeById(COLOR_PRIMARY_BG);
    AS_WIDGET(menu)->style.attributes = STYLE_BOLD;
    menu->style_selected = menu->base.style;
    menu->style_selected.attributes = STYLE_UNDERLINE;

}

Menu *Menu_Create(MenuEntry *entries, size_t entry_count, const char *title, Callback on_close) {
    Menu *self = malloc(sizeof(Menu));
    if (!self) {
        logFatal("Cannot allocate memory for MainMenu.");
    }
    Menu_Init(self, entries, entry_count, title, on_close);
    return self;
}
