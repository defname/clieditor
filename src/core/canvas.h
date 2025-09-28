/**
 * canvas.h
 * Abstracts a canvas to write on.
 */
#ifndef CANVAS_H
#define CANVAS_H

#include <stddef.h>
#include <stdint.h>
#include "utf8.h"
#include "utf8string.h"
#include "cell.h"

typedef struct _Canvas {
    Cell *buffer;
    size_t size;
    size_t capacity;
    int width;
    int height;
    int cursor_x;
    int cursor_y;
    Style current_style;
} Canvas;

void Canvas_Init(Canvas *canvas, int width, int height);
void Canvas_Deinit(Canvas *canvas);
void Canvas_Resize(Canvas *canvas, int width, int heigth);  // Reallocates memory for the buffer.

void Canvas_ClipTo(Canvas *canvas, Canvas *target, int x, int y);

// The following functions are buffered. All changes are drawn by Canvas_Draw()
void Canvas_Clear(Canvas *canvas);

void Canvas_MoveCursor(Canvas *canvas, int col, int row);
void Canvas_HideCursor(Canvas *canvas);
void Canvas_ShowCursor(Canvas *canvas);
void Canvas_PutChar(Canvas *canvas, UTF8Char c);  // put c to the current position and move the cursor
void Canvas_Write(Canvas *canvas, const UTF8String *s);  // write s to the current position and move the cursor

#endif