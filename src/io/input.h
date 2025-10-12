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
    KEY_NONE,
    KEY_CHAR,
    KEY_ESC,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_DELETE,
    KEY_INSERT,
} KeyCode;

#define KEY_MOD_ALT     1
#define KEY_MOD_CTRL    2
#define KEY_MOD_SHIFT   4

typedef struct _InputEvent {
    KeyCode key;
    UTF8Char ch;
    uint8_t mods;
} InputEvent;

bool InputEvent_IsValid(const InputEvent *ev);

void Input_Init();
void Input_Deinit();
InputEvent Input_Read();


#endif