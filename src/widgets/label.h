#ifndef LABEL_H
#define LABEL_H

#include "core/widget.h"
#include "core/utf8string.h"

typedef struct {
    UTF8String text;
} LabelData;


// "Konstruktor" für ein Label-Widget
Widget* Label_Create(Widget *parent, const char* text);

#endif // LABEL_H