/**
 * screen.h
 * Abstracts the screen of the terminal.
 * It provides a canvas to write/draw on.
 */
#ifndef SCREEN_H
#define SCREEN_H

#include <stddef.h>
#include <stdint.h>

#include "canvas.h"

typedef void (*ResizeCallback)(int new_width, int new_height);

typedef struct _Screen {
    Canvas canvas;
    ResizeCallback onResize;
} Screen;

extern Screen screen;

void Screen_Init(ResizeCallback onResize);  // call after Terminal_Init()!!!
void Screen_Deinit();

void Screen_HideCursor();
void Screen_ShowCursor();

void Screen_Draw();

int Screen_GetWidth();
int Screen_GetHeight();

#endif