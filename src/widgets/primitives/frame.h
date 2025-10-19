/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include "display/widget.h"
#include "common/string.h"


typedef struct {
    Widget base;
    Widget *container;  //< This need to be used as parent when adding children to the Frame
    uint32_t charset[8];
} Frame;

#define AS_FRAME(w) ((Frame*)(w))

void Frame_Init(Frame *self, Widget *parent);
Frame *Frame_Create(Widget *parent);

void Frame_SetBorderStyle(Frame *self, unsigned char style);
void Frame_SetBoxDrawingCharacters(Frame *self, const char *chars);

#endif