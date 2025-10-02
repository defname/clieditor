#include "app.h"

#include <stdlib.h>
#include "label.h"
#include "display/canvas.h"
#include "common/logging.h"

Widget app;  // global widget

static void app_destroy(Widget *self) {
    free(self->data);
    self->data = NULL;
}

static void app_handle_resize(Widget *self, int new_parent_width, int new_parent_height) {
    self->width = new_parent_width;
    self->height = new_parent_height;
}



static WidgetOps app_ops = {
    .draw = NULL,
    .destroy = app_destroy,
    .on_resize = app_handle_resize,
    .on_input = NULL,
};

void App_Init(int width, int height) {
    Widget_Init(&app, NULL, &app_ops);
    AppData *data = malloc(sizeof(AppData));
    data->focus = NULL;
    app.data = data;

    App_onParentResize(width, height);
}

void App_Deinit() {
    Widget_Deinit(&app);
}

void App_AddChild(Widget *child) {
    Widget_AddChild(&app, child);
}

void App_RemoveChild(Widget *child) {
    if (!child) {
        return;
    }
    AppData *data = (AppData*)app.data;
    if (!data) {
        logError("App has no data.");
        return;
    }
    if (App_HasFocus() == child) {
        App_ClearFocus();
    }
    Widget_RemoveChild(&app, child);
}

void App_Draw(Canvas *canvas) {
    Widget_Draw(&app, canvas);
}

void App_onParentResize(int new_parent_width, int new_parent_height) {
    Widget_onParentResize(&app, new_parent_width, new_parent_height);
}

void App_SetFocus(Widget *widget) {
    if (!widget) {
        return;
    }
    AppData *data = (AppData*)app.data;
    if (!data) {
        logError("App has no data.");
        return;
    }
    App_ClearFocus();
    data->focus = widget;
    if (widget->ops && widget->ops->on_focus) {
        widget->ops->on_focus(widget);
    }
}

void App_ClearFocus() {
    AppData *data = (AppData*)app.data;
    if (!data) {
        logError("App has no data.");
        return;
    }
    // if a Widget has focus blur it first
    Widget *focus = App_HasFocus();
    if (focus && focus->ops && focus->ops->on_blur) {
        focus->ops->on_blur(focus);
    }
    data->focus = NULL;
}

Widget *App_HasFocus() {
    AppData *data = (AppData*)app.data;
    if (!data) {
        logError("App has no data.");
        return NULL;
    }
    return data->focus;
}
