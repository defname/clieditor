#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include "display/widget.h"
#include "common/utf8string.h"

typedef struct {
    Widget base;

    UTF8String text;
} BottomBar;


void BottomBar_Init(BottomBar *self, Widget *parent);
BottomBar* BottomBar_Create(Widget *parent);

#endif