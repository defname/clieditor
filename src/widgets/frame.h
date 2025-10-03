#ifndef FRAME_H
#define FRAME_H

#include "display/widget.h"

Widget *Frame_Create(Widget *parent);

// This might be not usedful, since Widget_AddChild() is automatically called
// by Widget_Create(parent).
// Just make sure to use frame->data->container as parent when adding
// children to a frame
void Frame_AddChild(Widget *self, Widget *child);

void Frame_SetBorderStyle(Widget *self, unsigned char style);
void Frame_SetBoxDrawingCharacters(Widget *self, const char *chars);

#endif