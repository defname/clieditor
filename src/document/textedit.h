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
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "textbuffer.h"
#include "textlayout.h"

typedef struct _TextEdit {
    TextBuffer *tb;
    TextLayout *tl;
} TextEdit;

void TextEdit_Init(TextEdit *te, TextBuffer *tb, TextLayout *tls);
void TextEdit_Deinit(TextEdit *te);

// --- Cursor movement ---
void TextEdit_MoveLeft(TextEdit *te);
void TextEdit_MoveRight(TextEdit *te);
void TextEdit_MoveUp(TextEdit *te);
void TextEdit_MoveDown(TextEdit *te);

// --- Editing ---
void TextEdit_InsertChar(TextEdit *te, UTF8Char ch);
void TextEdit_DeleteChar(TextEdit *te);      // delete at cursor
void TextEdit_Backspace(TextEdit *te);       // delete before cursor
void TextEdit_Newline(TextEdit *te);

// --- Optional convenience ---
void TextEdit_InsertString(TextEdit *te, UTF8String *string);  // string should not contain newlines!

#endif