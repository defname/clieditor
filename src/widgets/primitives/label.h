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
#ifndef LABEL_H
#define LABEL_H

#include "display/widget.h"
#include "common/utf8string.h"

typedef struct {
    Widget base;
    UTF8String text;
} Label;

#define AS_LABEL(w) ((Label*)(w))

void Label_Init(Label *self, Widget *parent, const char *text);
Label* Label_Create(Widget *parent, const char* text);

#endif