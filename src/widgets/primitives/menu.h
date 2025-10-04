#ifndef MAINMENU_H
#define MAINMENU_H

#include "display/widget.h"
#include "common/callback.h"

#define MENU_BORDER_X 3
#define MENU_BORDER_Y 1

typedef struct _MenuEntry {
    const char *text;
    Callback callback;
} MenuEntry;

typedef struct {
    Widget base;
    MenuEntry *entries;
    size_t entry_count;
    size_t selected_entry;
    Callback on_close;
} Menu;

#define AS_MENU(w) ((Menu *)(w))

void Menu_Init(Menu *menu, MenuEntry *entries, size_t entry_count, Callback on_close);
Menu *Menu_Create(MenuEntry *entries, size_t entry_count, Callback on_close);

#endif