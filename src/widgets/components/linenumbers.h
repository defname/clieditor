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
#ifndef LINENUMBERS_H
#define LINENUMBERS_H

#include "display/widget.h"
#include "document/textlayout.h"


typedef struct _LineNumbers {
    Widget base;

    TextLayout *tl;
    int first_number;
} LineNumbers;

#define AS_LINENUMBERS(w) ((LineNumbers *)(w))

void LineNumbers_Init(LineNumbers *ln, Widget *parent, TextLayout *tl);
LineNumbers *LineNumbers_Create(Widget *parent, TextLayout *tl);

#endif