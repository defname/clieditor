#ifndef EDITOR_H
#define EDITOR_H

#include "core/widget.h"
#include "core/textbuffer.h"

typedef struct {
    TextBuffer *tb;
    Line *first_line;
} EditorData;

Widget *Editor_Create(Widget *parent, TextBuffer *tb);

#endif