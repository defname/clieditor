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
#ifndef LINE_H
#define LINE_H

#include "common/utf8string.h"

#define LINE_POSITION_STEP 100

typedef struct _Line {
    UTF8String text;

    int position;

    struct _Line *prev;
    struct _Line *next;
} Line;

Line *Line_Create();
void Line_Destroy(Line *l);
void Line_InsertBefore(Line *line, Line *new_line);
void Line_InsertAfter(Line *line, Line *new_line);
void Line_Delete(Line *line);

#endif