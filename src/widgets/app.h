#ifndef APP_H
#define APP_H

#include "display/widget.h"

typedef struct {
    Widget base;
} App;

extern App app;

void App_Init(int width, int height);
void App_Deinit();

bool App_HandleInput(EscapeSequence key, UTF8Char ch);
void App_Draw(Canvas *canvas);

void App_onParentResize(int new_parent_width, int new_parent_height);

#endif