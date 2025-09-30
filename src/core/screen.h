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

/**
 * @brief Hides the terminal cursor.
 *
 * Sends an ANSI escape code to the terminal to make the cursor invisible.
 */
void Screen_HideCursor();

/**
 * @brief Shows the terminal cursor.
 *
 * Sends an ANSI escape code to the terminal to make the cursor visible.
 */
void Screen_ShowCursor();

/**
 * @brief Draws the content of the screen's canvas to the terminal.
 *
 * This function iterates through the canvas buffer and updates only the cells
 * that have changed since the last draw call, optimizing terminal output.
 * It handles cursor positioning and style changes.
 */
void Screen_Draw();

/**
 * @brief Clears the entire terminal screen.
 *
 * Sends an ANSI escape code to clear the terminal display.
 */
void Screen_Clear();

/**
 * @brief Returns the current width of the screen's canvas in columns.
 */
int Screen_GetWidth();

/**
 * @brief Returns the current height of the screen's canvas in rows.
 */
int Screen_GetHeight();

#endif