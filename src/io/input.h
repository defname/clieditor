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
 * The stream of multi byte characters and escape sequences is recognized and
 * packed into InputEvent structures.
 * The only needed function is
 *   Input_Read()
 * which should be called at the beginning of every iteration of the main loop.
 * The returned struct contains a keycode or a character (not both) together
 * with optional modification flags.
 * Note that the KEY_MOD_SHIFT will only be set together with a keycode. Characters
 * or symbols will just be in uppercase.
 * (like Shift+ArrowLeft will result in key=KEY_LEFT and mod & KEY_MOD_SHIFT,
 * but Shift+"a" is just "A")
 */
#ifndef INPUT_H
#define INPUT_H

#include "common/utf8.h"

#define INPUT_BUFFER_SIZE   24

typedef enum {
    KEY_NONE,
    // character stored in InputEvent.ch
    KEY_CHAR,
    // for convinience, also delivered as character
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_ESC,
    // CSI escape sequences
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

void Input_Init();      //< deprecated
void Input_Deinit();    //< deprecated
InputEvent Input_Read();


#endif