#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include "display/widget.h"
#include "common/utf8string.h"

typedef struct {
    int cursor_x;
    int cursor_y;

    UTF8String text;
} BottomBarData;

Widget *BottomBar_Create(Widget *parent);

#endif