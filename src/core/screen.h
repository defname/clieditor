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

typedef struct _Screen {
    Canvas canvas;
} Screen;

extern Screen screen;

void Screen_Init();  // call after Terminal_Init()!!!
void Screen_Deinit();
void Screen_onResize();  // Reallocates memory for the buffer. Need to be called AFTER Terminal_Update()

void Screen_HideCursor();
void Screen_ShowCursor();

void Screen_Draw();

#endif