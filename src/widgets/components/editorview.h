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
#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include "display/widget.h"
#include "editor.h"
#include "linenumbers.h"

typedef struct _EditorView {
    Widget base;
    Editor *editor;
    LineNumbers *line_numbers;
} EditorView;

#define AS_EDITOR_VIEW(w) ((EditorView *)(w))

void EditorView_Init(EditorView *view, Widget *parent, TextBuffer *tb);
EditorView *EditorView_Create(Widget *parent, TextBuffer *tb);


#endif