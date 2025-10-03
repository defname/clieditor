#ifndef EDITOR_H
#define EDITOR_H

#include "display/widget.h"
#include "document/textbuffer.h"
#include "io/timer.h"

typedef struct {
    Widget base;
    TextBuffer *tb;
    Line *first_line;
    uint8_t cursor_timer;
    bool cursor_visible;
} Editor;

#define AS_EDITOR(w) ((Editor*)(w))

void Editor_Init(Editor *self, Widget *parent, TextBuffer *tb);
Editor *Editor_Create(Widget *parent, TextBuffer *tb);

#endif