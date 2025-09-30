#ifndef EDITOR_H
#define EDITOR_H

#include "core/widget.h"
#include "core/textbuffer.h"
#include "core/timer.h"

typedef struct {
    TextBuffer *tb;
    Line *first_line;
    uint8_t timer;
} EditorData;

Widget *Editor_Create(Widget *parent, TextBuffer *tb);

#endif