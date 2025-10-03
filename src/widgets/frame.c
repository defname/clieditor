#include "frame.h"

#include <string.h>

#include "common/logging.h"
#include "common/utf8string.h"

// 0:┌ 1:─ 2:┒ 3:│ 4:┕ 5:┛ 6:┃ 7:━
static const char *box_drawing_chars[] = {
    "┌─┐│└┘│─",
    "┏━┓┃┗┛┃━",
    "╔═╗║╚╝║═"
};

typedef struct {
    Widget *container;
    UTF8String charset;
} FrameData;

static void frame_destroy(Widget *self) {
    FrameData *data = self->data;
    if (data) {
        UTF8String_Deinit(&data->charset);
        free(data);
        self->data = NULL;
    }
}

static void frame_on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    (void)self;
    (void)new_parent_width;
    (void)new_parent_height;
}

static void frame_draw(const Widget *self, Canvas *canvas) {
    FrameData *data = self->data;
    UTF8Char *border_chars = data->charset.chars;

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

Widget *Frame_Create(Widget *parent) {
    Widget *self = Widget_Create(parent, &frame_ops);
    FrameData *data = malloc(sizeof(FrameData));
    if (!data) {
        logFatal("Cannot allocate memory for FrameData.");
    }

    UTF8String_Init(&data->charset);

    self->style.bg = 19;

    data->container = Widget_Create(self, &container_ops);
    data->container->x = 1;
    data->container->y = 1;
    self->data = data;

    Frame_SetBorderStyle(self, 0);  // needs self->data to be set before
    return self;
}

void Frame_AddChild(Widget *self, Widget *child) {
    FrameData *data = self->data;
    if (!data) {
        logFatal("Frame has no data.");
    }
    Widget_AddChild(data->container, child);
}

void Frame_SetBorderStyle(Widget *self, unsigned char style) {
    if (style > (sizeof(box_drawing_chars) / sizeof(box_drawing_chars[0]))) {
        logWarn("style out of bounds. Border style not set.");
        return;
    }
    if (!self || !self->data) {
        logError("Invalid widget.");
        return;
    }
    Frame_SetBoxDrawingCharacters(self, box_drawing_chars[style]);
}

void Frame_SetBoxDrawingCharacters(Widget *self, const char *chars) {
    if (!self || !self->data) {
        logError("Invalid widget.");
        return;
    }
    FrameData *data = self->data;
    UTF8String_FromStr(&data->charset, chars, strlen(chars));
    if (data->charset.length != 8) {
        logError("Invalid charset length.");
        Frame_SetBorderStyle(self, 0);
    }
}