/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * screen.h
 * Abstracts the screen of the terminal.
 * It provides a canvas to write/draw on.
 */
#ifndef SCREEN_H
#define SCREEN_H

#include <stddef.h>
#include <stdint.h>

#include "display/canvas.h"

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