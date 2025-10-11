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
    Widget_Init(AS_WIDGET(&app), NULL, &app_ops);
    Widget_Focus(AS_WIDGET(&app));
    App_onParentResize(width, height);
}

void App_Deinit() {
    for (int i=0; i<app.base.children_count; i++) {
        Widget_Destroy(app.base.children[i]);
        app.base.children[i] = NULL;
    }
    Widget_Deinit(&app.base);
}

bool App_HandleInput(EscapeSequence key, UTF8Char ch) {
    return Widget_HandleInput(&app.base, key, ch);
}

void App_Draw(Canvas *canvas) {
    Widget_Draw(&app.base, canvas);
}

void App_onParentResize(int new_parent_width, int new_parent_height) {
    Widget_onParentResize(&app.base, new_parent_width, new_parent_height);
}
