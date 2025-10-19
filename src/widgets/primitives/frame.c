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
#include "frame.h"

#include <string.h>
#include <stdlib.h>

#include "common/logging.h"
#include "common/string.h"
#include "common/utf8_helper.h"
#include "common/colors.h"

// 0:┌ 1:─ 2:┒ 3:│ 4:┕ 5:┛ 6:┃ 7:━
static const char *box_drawing_chars[] = {
    "┌─┐│└┘│─",
    "┏━┓┃┗┛┃━",
    "╔═╗║╚╝║═"
};

static void frame_destroy(Widget *self) {
    AS_FRAME(self)->container = NULL;
}

static void frame_on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    (void)self;
    (void)new_parent_width;
    (void)new_parent_height;
}

static void frame_draw(const Widget *self, Canvas *canvas) {
    uint32_t *border_chars = AS_FRAME(self)->charset;

    if (self->width < 2 || self->height < 2) {
        logWarn("Frame is too small to render.");
        return;
    }

    // Draw a border

    // top border
    Canvas_PutChar(canvas, border_chars[0]);
    for (int i=0; i<self->width-2; i++) {
        Canvas_PutChar(canvas, border_chars[1]); 
    }
    Canvas_PutChar(canvas, border_chars[2]);

    // sides
    for (int i=1; i<self->height-1; i++) {
        Canvas_MoveCursor(canvas, 0, i);
        Canvas_PutChar(canvas, border_chars[3]);
        Canvas_MoveCursor(canvas, self->width-1, i);
        Canvas_PutChar(canvas, border_chars[6]);
    }

    // bottom border
    Canvas_MoveCursor(canvas, 0, self->height-1);
    Canvas_PutChar(canvas, border_chars[4]);
    for (int i=1; i<self->width-1; i++) {
        Canvas_PutChar(canvas, border_chars[7]);
    }
    Canvas_PutChar(canvas, border_chars[5]);
}

static void container_on_resize(Widget *container, int parent_w, int parent_h) {
    container->width = parent_w - 2;
    container->height = parent_h - 2;
}

WidgetOps frame_ops = {
    .destroy = frame_destroy,
    .on_resize = frame_on_resize,
    .draw = frame_draw,
};

WidgetOps container_ops = {
    .on_resize = container_on_resize,
};

void Frame_Init(Frame *self, Widget *parent) {
    Widget_Init(&self->base, parent, &frame_ops);
    Frame_SetBorderStyle(self, 0);
    self->container->x = 1;
    self->container->y = 1;
}

Frame *Frame_Create(Widget *parent) {
    Frame *self = malloc(sizeof(Frame));
    if (!self) {
        logFatal("Cannot allocate memory for Frame.");
    }
    Frame_Init((Frame*)self, parent);
    return self;
}

void Frame_SetBorderStyle(Frame *self, unsigned char style) {
    if (style >= (sizeof(box_drawing_chars) / sizeof(box_drawing_chars[0]))) {
        logWarn("style out of bounds. Border style not set.");
        return;
    }
    Frame_SetBoxDrawingCharacters(self, box_drawing_chars[style]);
}

void Frame_SetBoxDrawingCharacters(Frame *self, const char *chars) {
    if (!chars) {
        logError("Charset is NULL.");
        Frame_SetBorderStyle(self, 0);
        return;
    }
    String tmp = String_FromCStr(chars, strlen(chars));
    if (String_Length(&tmp) != 8) {
        logError("Invalid charset length.");
        Frame_SetBorderStyle(self, 0);
    }
    for (int i=0; i<8; i++) {
        self->charset[i] = utf8_to_codepoint(String_GetChar(&tmp, i));
    }
    String_Deinit(&tmp);
}