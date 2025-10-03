#ifndef FRAME_H
#define FRAME_H

#include "display/widget.h"
#include "common/utf8string.h"


typedef struct {
    Widget base;
    Widget *container;  //< This need to be used as parent when adding children to the Frame
    UTF8String charset;
} Frame;

#define AS_FRAME(w) ((Frame*)(w))

void Frame_Init(Frame *self, Widget *parent);
Frame *Frame_Create(Widget *parent);

void Frame_SetBorderStyle(Frame *self, unsigned char style);
void Frame_SetBoxDrawingCharacters(Frame *self, const char *chars);

#endif