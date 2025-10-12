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
#ifndef EDITOR_H
#define EDITOR_H

#include "display/widget.h"
#include "document/textbuffer.h"
#include "document/textlayout.h"
#include "document/textedit.h"
#include "document/textselection.h"
#include "io/timer.h"

typedef enum {
    EDITOR_MODE_INPUT,
    EDITOR_MODE_SELECT
} EditorMode;

typedef struct {
    Widget base;

    TextBuffer *tb;
    TextLayout tl;
    TextEdit te;
    TextSelection ts;

    EditorMode mode;
    
    uint8_t cursor_timer;
    bool cursor_visible;
} Editor;

#define AS_EDITOR(w) ((Editor*)(w))

void Editor_Init(Editor *self, Widget *parent, TextBuffer *tb);
Editor *Editor_Create(Widget *parent, TextBuffer *tb);

#endif