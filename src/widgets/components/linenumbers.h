#ifndef LINENUMBERS_H
#define LINENUMBERS_H

#include "display/widget.h"
#include "document/textlayout.h"

#define LINENUMBERS_MIN_DIGITS 3

typedef struct _LineNumbers {
    Widget base;

    TextLayout *tl;
    int first_number;
} LineNumbers;

#define AS_LINENUMBERS(w) ((LineNumbers *)(w))

void LineNumbers_Init(LineNumbers *ln, Widget *parent, TextLayout *tl);
LineNumbers *LineNumbers_Create(Widget *parent,TextLayout *tl);

#endif