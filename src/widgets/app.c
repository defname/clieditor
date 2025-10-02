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
    .handle_resize = app_handle_resize,
    .handle_input = NULL,
};

void App_Init(int width, int height) {
    Widget_Init(&app, NULL, &app_ops);
    AppData *data = malloc(sizeof(AppData));
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
    Widget_RemoveChild(&app, child);
}

void App_Draw(Canvas *canvas) {
    Widget_Draw(&app, canvas);
}

void App_onParentResize(int new_parent_width, int new_parent_height) {
    Widget_onParentResize(&app, new_parent_width, new_parent_height);
}

void App_HandleInput(EscapeSequence key, UTF8Char ch) {
    Widget_HandleInput(&app, key, ch);
}
