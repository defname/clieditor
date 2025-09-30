#ifndef LABEL_H
#define LABEL_H

#include "display/widget.h"
#include "common/utf8string.h"

typedef struct {
    UTF8String text;
} LabelData;


// "Konstruktor" für ein Label-Widget
Widget* Label_Create(Widget *parent, const char* text);

#endif // LABEL_H