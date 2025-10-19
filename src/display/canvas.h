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
 * canvas.h
 * Abstracts a canvas to write on.
 */
#ifndef CANVAS_H
#define CANVAS_H

#include <stddef.h>
#include <stdint.h>
#include "common/string.h"
#include "cell.h"

/**
 * @brief Represents a drawing surface (canvas) for terminal output.
 *
 * A Canvas manages a 2D grid of `Cell` objects, each representing a character
 * and its style at a specific terminal coordinate. It provides functions for
 * initialization, resizing, drawing characters and strings, and managing
 * a drawing cursor.
 */
typedef struct _Canvas {
    Cell *buffer;    /**< Pointer to the array of Cell objects representing the canvas content. */
    size_t size;     /**< The current number of active cells in the buffer (width * height). */
    size_t capacity; /**< The total allocated capacity for the buffer. */
    int width;       /**< The width of the canvas in columns. */
    int height;      /**< The height of the canvas in rows. */
    int cursor_x;    /**< The current column position of the drawing cursor. */
    int cursor_y;    /**< The current row position of the drawing cursor. */
    Style current_style; /**< The style to be applied to newly drawn characters. */
} Canvas;

/**
 * @brief Initializes a Canvas object.
 *
 * Allocates memory for the canvas buffer and sets initial dimensions and state.
 *
 * @param canvas A pointer to the Canvas object to initialize.
 * @param width The initial width of the canvas.
 * @param height The initial height of the canvas.
 */
void Canvas_Init(Canvas *canvas, int width, int height);

/**
 * @brief Deinitializes a Canvas object.
 *
 * Frees the memory allocated for the canvas buffer.
 *
 * @param canvas A pointer to the Canvas object to deinitialize.
 */
void Canvas_Deinit(Canvas *canvas);

/**
 * @brief Resizes the canvas.
 *
 * Reallocates memory for the buffer if necessary and updates the canvas dimensions.
 *
 * @param canvas A pointer to the Canvas object to resize.
 * @param width The new width of the canvas.
 * @param height The new height of the canvas.
 */
void Canvas_Resize(Canvas *canvas, int width, int height);

/**
 * @brief Clips content from one canvas onto another.
 *
 * Copies the content of the source `canvas` to a `target` canvas,
 * starting at the specified (x, y) coordinates on the target.
 * Only cells within the bounds of the target canvas are updated.
 *
 * @param canvas The source Canvas to clip from.
 * @param target The target Canvas to clip to.
 * @param x The starting column on the target canvas.
 * @param y The starting row on the target canvas.
 */
void Canvas_ClipTo(Canvas *canvas, Canvas *target, int x, int y);

/**
 * @brief Clears the entire canvas.
 *
 * Sets all cells in the canvas buffer to a blank character with default style
 * and marks them as changed.
 *
 * @param canvas A pointer to the Canvas object to clear.
 */
void Canvas_Clear(Canvas *canvas);

/**
 * @brief Fills the canvas with a specified style.
 *
 * Sets the style of all cells in the canvas to the given `style`.
 * Only cells whose character is not blank or whose style differs from the new style
 * are marked as changed.
 *
 * @param canvas A pointer to the Canvas object to fill.
 * @param style The Style to apply to all cells.
 */
void Canvas_Fill(Canvas *canvas, Style style);

/**
 * @brief Moves the drawing cursor to a specific position.
 *
 * Sets the `cursor_x` and `cursor_y` properties of the canvas.
 *
 * @param canvas A pointer to the Canvas object.
 * @param col The target column for the cursor.
 * @param row The target row for the cursor.
 */
void Canvas_MoveCursor(Canvas *canvas, int col, int row);

/**
 * @brief Puts a single UTF8 character at the current cursor position.
 *
 * Writes the character `c` to the cell at `canvas->cursor_x`, `canvas->cursor_y`
 * using `canvas->current_style`. The cursor is then advanced to the right.
 *
 * @param canvas A pointer to the Canvas object.
 * @param c The UTF8Char to put on the canvas.
 */
void Canvas_PutChar(Canvas *canvas, uint32_t cp);

/**
 * @brief Writes a UTF8String to the canvas starting at the current cursor position.
 *
 * Iterates through the characters of the string `s` and calls `Canvas_PutChar`
 * for each, effectively writing the string horizontally.
 *
 * @param canvas A pointer to the Canvas object.
 * @param s A pointer to the UTF8String to write.
 */
void Canvas_Write(Canvas *canvas, const String *s);

#endif