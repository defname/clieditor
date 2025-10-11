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
 * utf8.h
 * Function to read and write UTF8 characters and strings.
 */
#ifndef UTF8_H
#define UTF8_H

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#include "common/buffer.h"

#define INVALID_CODEPOINT 0xFFFFFFFF


typedef struct _UTF8Char {
    unsigned char bytes[4];
    char length;
} UTF8Char;

extern const UTF8Char utf8_space;
extern const UTF8Char utf8_invalid;

UTF8Char UTF8_GetCharFromString(const char *s); // Read an UTF8 character from a string

UTF8Char UTF8_ReadCharFromBuf(Buffer *buf); 

ssize_t UTF8_PutChar(int fd, UTF8Char ch);      // Write an UTF character to fd (return the number of bytes written or -1 on failure)
bool UTF8_Equal(UTF8Char a, UTF8Char b);        // return true if a and b are equal
bool UTF8_EqualToChar(UTF8Char a, char b);      // return true if a and b are equal

uint32_t UTF8_ToCodepoint(UTF8Char ch);
bool UTF8_IsPrintable(UTF8Char ch);

bool UTF8_IsSpace(UTF8Char ch);
bool UTF8_IsASCII(UTF8Char ch);
char UTF8_AsASCII(UTF8Char ch);
int UTF8_GetWidth(UTF8Char ch);     //< return the display width of ch

#endif