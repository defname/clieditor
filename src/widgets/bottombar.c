#include "bottombar.h"

#include <stdlib.h>
#include "label.h"
#include "display/canvas.h"
#include "common/logging.h"
#include "common/config.h"

void bottombar_draw(const Widget *self, Canvas *canvas) {
    (void)self;
    (void)canvas;
}

void bottombar_destroy(Widget *self) {
    if (self && self->data) {
        free(self->data);
        self->data = NULL;
    }
}

void bottombar_handle_resize(Widget *self, int new_parent_width, int new_parent_height) {
    if (!self->parent) {
        logDebug("BottomBar has no parent.");
        return;
    }
    self->x = 0;
    self->y = new_parent_height - 1;
    self->width = new_parent_width;
    self->height = 1;
}

static WidgetOps bottombar_ops = {
    .draw = bottombar_draw,
    .destroy = bottombar_destroy,
    .handle_resize = bottombar_handle_resize,
    .handle_input = NULL,
};

Widget *BottomBar_Create(Widget *parent) {
    Widget *self = Widget_Create(parent, &bottombar_ops);
    BottomBarData *data = malloc(sizeof(BottomBarData));
    self->data = data;
    self->style.bg = 21;
    Widget *label = Label_Create(self, Config_GetFilename());
    label->x = 0;
    label->y = 0;
    label->width = FILENAME_MAX_LENGTH;
    label->height = 1;

    Widget_onParentResize(self, parent->width, parent->height);
    return self;
}