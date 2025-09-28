#ifndef EDITOR_H
#define EDITOR_H

#include "core/widget.h"
#include "core/utf8string.h"

typedef struct {
    UTF8String *text;
    size_t line_no;
} EditorData;

Widget *Editor_Create(Widget *parent, UTF8String *text);

#endif