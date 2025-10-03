#ifndef LABEL_H
#define LABEL_H

#include "display/widget.h"
#include "common/utf8string.h"

typedef struct {
    Widget base;
    UTF8String text;
} Label;

#define AS_LABEL(w) ((Label*)(w))

void Label_Init(Label *self, Widget *parent, const char *text);
Label* Label_Create(Widget *parent, const char* text);

#endif