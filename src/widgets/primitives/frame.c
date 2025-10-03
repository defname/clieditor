#include "frame.h"

#include <string.h>
#include <stdlib.h>

#include "common/logging.h"
#include "common/utf8string.h"

// 0:┌ 1:─ 2:┒ 3:│ 4:┕ 5:┛ 6:┃ 7:━
static const char *box_drawing_chars[] = {
    "┌─┐│└┘│─",
    "┏━┓┃┗┛┃━",
    "╔═╗║╚╝║═"
};

static void frame_destroy(Widget *self) {
    UTF8String_Deinit(&(AS_FRAME(self))->charset);
    AS_FRAME(self)->container = NULL;
}

static void frame_on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    (void)self;
    (void)new_parent_width;
    (void)new_parent_height;
}

static void frame_draw(const Widget *self, Canvas *canvas) {
    UTF8Char *border_chars = AS_FRAME(self)->charset.chars;

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
    UTF8String_Init(&self->charset);
    Frame_SetBorderStyle(self, 0);
    self->base.style.bg = 32;
    self->container = Widget_Create(AS_WIDGET(self), &container_ops);
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
    UTF8String_FromStr(&self->charset, chars, strlen(chars));
    if (self->charset.length != 8) {
        logError("Invalid charset length.");
        Frame_SetBorderStyle(self, 0);
    }
}