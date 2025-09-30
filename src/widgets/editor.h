#ifndef EDITOR_H
#define EDITOR_H

#include "core/widget.h"
#include "core/textbuffer.h"
#include "core/timer.h"

typedef struct {
    TextBuffer *tb;
    Line *first_line;
    uint8_t cursor_timer;
    bool cursor_visible;
} EditorData;

Widget *Editor_Create(Widget *parent, TextBuffer *tb);

#endif