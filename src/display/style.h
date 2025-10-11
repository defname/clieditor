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
#ifndef STYLE_H
#define STYLE_H

#include <stdint.h>
#include <stdbool.h>

#define STYLE_NONE      0
#define STYLE_BOLD      1
#define STYLE_UNDERLINE 2

typedef struct {
    uint8_t fg; // foreground color (ANSI-Code 0-255)
    uint8_t bg; // background color (ANSI-Code 0-255)
    uint16_t attributes; // Bit-Flags für fett, unterstrichen etc.
} Style;

bool Style_Equal(const Style *a, const Style *b);

#endif