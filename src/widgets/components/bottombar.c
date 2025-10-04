#include "bottombar.h"

#include <stdlib.h>
#include "widgets/primitives/label.h"
#include "display/canvas.h"
#include "common/logging.h"
#include "common/config.h"
#include "common/colors.h"

void bottombar_draw(const Widget *self, Canvas *canvas) {
    (void)self;
    (void)canvas;
}

void bottombar_destroy(Widget *self) {
    (void)self;
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
    .on_resize = bottombar_handle_resize,
    .on_input = NULL,
};

void BottomBar_Init(BottomBar *self, Widget *parent) {
    Widget_Init(AS_WIDGET(self), parent, &bottombar_ops);
    self->base.x = 0;
    self->base.y = 0;
    self->base.width = FILENAME_MAX_LENGTH;
    self->base.height = 1;

    self->base.style.bg = Color_GetCodeById(COLOR_PRIMARY_BG);
    self->base.style.fg = Color_GetCodeById(COLOR_PRIMARY_FG);

    Label_Create(AS_WIDGET(self), Config_GetFilename());
}

BottomBar *BottomBar_Create(Widget *parent) {
    BottomBar *self = malloc(sizeof(BottomBar));
    if (!self) {
        logFatal("Cannot allocate memory for BottomBar.");
    }
    BottomBar_Init(self, parent);
    return self;
}