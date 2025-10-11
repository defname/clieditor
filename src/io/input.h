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
 * input.h
 * Read and buffer input to recognize ESC sequences.
 * It's important to empty the buffer after Input_Read() by calling
 * Input_GetChar() repeatedly!
 * 
 * Usage:
 * 
 * EscapeSequence e = Input_Read()  // read from terminal
 * 
 * while ((UTF8Char ch = Input_GetChar()).length > 0) {
 *     handleInputChar(ch);
 * }
 * 
 * Expected behavier:
 * Inpu_Read() returns the escape sequence that was read or ESC_NONE. If ESC_NONE is returned
 * all symbols that were part of a started sequence are discarded.
 * ESC will never occure in the output of Input_GetChar()
 */
#ifndef INPUT_H
#define INPUT_H

#include "common/utf8.h"

#define INPUT_BUFFER_SIZE   24

typedef enum {
    ESC_ESCAPE,
    ESC_CURSOR_UP,
    ESC_CURSOR_DOWN,
    ESC_CURSOR_RIGHT,
    ESC_CURSOR_LEFT,
    ESC_SHIFT_CURSOR_UP,
    ESC_SHIFT_CURSOR_DOWN,
    ESC_SHIFT_CURSOR_RIGHT,
    ESC_SHIFT_CURSOR_LEFT,
    ESC_HOME,
    ESC_END,
    ESC_PAGE_UP,
    ESC_SHIFT_PAGE_UP,
    ESC_PAGE_DOWN,
    ESC_SHIFT_PAGE_DOWN,
    ESC_DELETE,
    ESC_NONE
} EscapeSequence;

#define KEY_ENTER       10
#define KEY_BACKSPACE   127
#define KEY_TAB         9


void Input_Init();
void Input_Deinit();
EscapeSequence Input_Read();
UTF8Char Input_GetChar();


#endif