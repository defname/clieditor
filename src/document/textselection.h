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
#ifndef TEXTSELECTION_H
#define TEXTSELECTION_H

#include "textbuffer.h"
#include "common/utf8string.h"


typedef struct _TextSelection {
    Line *start;
    int start_idx;
    Line *end;
    int end_idx;
} TextSelection;

void TextSelection_Init(TextSelection *ts);
void TextSelection_Deinit(TextSelection *ts);

void TextSelection_Select(TextSelection *ts, Line *line, int idx);
void TextSelection_Abort(TextSelection *ts);

bool TextSelection_IsSelected(TextSelection *ts, Line *line, int idx);

bool TextSelection_Started(TextSelection *ts);

void TextSelection_Begin(TextSelection *ts, Line *line, int idx);
void TextSelection_End(TextSelection *ts, Line *line, int idx);

TextSelection TextSelection_Ordered(const TextSelection *ts);

void TextSelection_Extract(TextSelection *ts, UTF8String *text);
void TextSelection_Delete(TextSelection *ts, TextBuffer *tb);

#endif