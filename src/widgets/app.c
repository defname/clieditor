#include "app.h"

#include <stdlib.h>
#include "display/canvas.h"
#include "common/logging.h"

App app;  // global widget

static void app_destroy(Widget *self) {
    (void)self;
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
    Widget_Init(&app.base, NULL, &app_ops);
    app.focus = NULL;
    App_onParentResize(width, height);
}

void App_Deinit() {
    Widget_Deinit(&app.base);
}

void App_RemoveChild(Widget *child) {
    if (!child) {
        return;
    }
    if (App_HasFocus() == child) {
        App_ClearFocus();
    }
    Widget_RemoveChild(AS_WIDGET(&app), child);
}

void App_Draw(Canvas *canvas) {
    Widget_Draw(&app.base, canvas);
}

void App_onParentResize(int new_parent_width, int new_parent_height) {
    Widget_onParentResize(&app.base, new_parent_width, new_parent_height);
}

void App_SetFocus(Widget *widget) {
    if (!widget) {
        return;
    }
    App_ClearFocus();
    app.focus = widget;
    if (widget->ops && widget->ops->on_focus) {
        widget->ops->on_focus(widget);
    }
}

void App_ClearFocus() {
    // if a Widget has focus blur it first
    Widget *focus = App_HasFocus();
    if (focus && focus->ops && focus->ops->on_blur) {
        focus->ops->on_blur(focus);
    }
    app.focus = NULL;
}

Widget *App_HasFocus() {
    return app.focus;
}
