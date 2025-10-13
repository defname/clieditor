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