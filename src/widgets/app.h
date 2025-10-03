#ifndef APP_H
#define APP_H

#include "display/widget.h"

typedef struct {
    Widget base;
    Widget *focus;
} App;

extern App app;

void App_Init(int width, int height);
void App_Deinit();

void App_RemoveChild(Widget *child);

void App_Draw(Canvas *canvas);

void App_onParentResize(int new_parent_width, int new_parent_height);

void App_SetFocus(Widget *widget);
void App_ClearFocus();
Widget *App_HasFocus();

#endif