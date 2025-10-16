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
#ifndef MAINMENU_H
#define MAINMENU_H

#include "display/widget.h"
#include "common/callback.h"
#include "common/utf8string.h"
#include "common/utf8.h"


#define MENU_BORDER_X 3
#define MENU_BORDER_Y 1

typedef struct _MenuEntry {
    const char *text;
    UTF8Char shortcut;
    Callback callback;
} MenuEntry;

typedef struct {
    Widget base;
    MenuEntry *entries;
    size_t entry_count;
    size_t selected_entry;
    UTF8String title;
    Callback on_close;
    Style style_selected;
} Menu;

#define AS_MENU(w) ((Menu *)(w))

void Menu_Init(Menu *menu, MenuEntry *entries, size_t entry_count, const char *title, Callback on_close);
Menu *Menu_Create(MenuEntry *entries, size_t entry_count, const char *title, Callback on_close);

#endif