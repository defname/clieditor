#include "app.h"

#include <stdlib.h>
#include "label.h"
#include "core/canvas.h"
#include "utils/logging.h"

void app_destroy(Widget *self) {
    free(self->data);
    self->data = NULL;
}

void app_handle_resize(Widget *self, int new_parent_width, int new_parent_height) {
    self->width = new_parent_width;
    self->height = new_parent_height;
}



static WidgetOps app_ops = {
    .draw = NULL,
    .destroy = app_destroy,
    .handle_resize = app_handle_resize,
    .handle_input = NULL,
};

Widget *App_Create(int width, int height) {
    Widget *self = Widget_Create(NULL, &app_ops);
    AppData *data = malloc(sizeof(AppData));
    self->data = data;

    Widget_onParentResize(self, width, height);

    return self;
}