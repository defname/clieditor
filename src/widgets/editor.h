#ifndef EDITOR_H
#define EDITOR_H

#include "display/widget.h"
#include "document/textbuffer.h"
#include "io/timer.h"

typedef struct {
    TextBuffer *tb;
    Line *first_line;
    uint8_t cursor_timer;
    bool cursor_visible;
} EditorData;

Widget *Editor_Create(Widget *parent, TextBuffer *tb);

#endif