#include "frame.h"

#include "common/logging.h"
#include "common/utf8string.h"

typedef struct {
    Widget *container;
} FrameData;

static void frame_destroy(Widget *self) {
    FrameData *data = self->data;
    if (data) {
        free(data);
        self->data = NULL;
    }
}

static void frame_on_resize(Widget *self, int new_parent_width, int new_parent_height) {
}

static void frame_draw(const Widget *self, Canvas *canvas) {
    // Draw a border
    UTF8String border_chars;
    UTF8String_Init(&border_chars);
    UTF8String_FromStr(&border_chars, "┌─┐│└┘", 6); // 0:┌, 1:─, 2:┐, 3:│, 4:└, 5:┘
    // top border
    Canvas_PutChar(canvas, border_chars.chars[0]);
    for (int i=0; i<self->width-2; i++) {
        Canvas_PutChar(canvas, border_chars.chars[1]); 
    }
    Canvas_PutChar(canvas, border_chars.chars[2]);
    // sides
    for (int i=0; i<self->height-2; i++) {
        Canvas_PutChar(canvas, border_chars.chars[3]);
        for (int j=0; j<self->width-2; j++) {
            Canvas_PutChar(canvas, utf8_space);
        }
        Canvas_PutChar(canvas, border_chars.chars[3]);
    }

    // bottom border
    Canvas_PutChar(canvas, border_chars.chars[4]);
    for (int i=0; i<self->width-2; i++) {
        Canvas_PutChar(canvas, border_chars.chars[1]);
    }
    Canvas_PutChar(canvas, border_chars.chars[5]);

    UTF8String_Deinit(&border_chars);
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
    data->container = Widget_Create(self, &container_ops);
    self->data = data;
    return self;
}
