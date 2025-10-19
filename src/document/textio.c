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
#include "textio.h"

#include "textedit.h"

void TextBuffer_LoadFromFile(TextBuffer *tb, File *file) {
    TextBuffer_ReInit(tb);
    Line *first = tb->current_line;
    Line *current = tb->current_line;
    String *line;
    while ((line = File_ReadLine(file)) != NULL) {
        String_Take(&current->text, line);
        String_Destroy(line);   // ownership was transfered, so no total destruction needed, but works anyway since NULL guards in String_Destroy()
        Line *newline = Line_Create();
        TextBuffer_InsertLineAfterCurrent(tb, newline);
        tb->current_line = newline;
        current = newline;
    }
    // current is now a last empty line which was not in the document
    // so delete it
    tb->current_line = first;  // change current line first
    if (tb->line_count > 1) {
        TextBuffer_DeleteLine(tb, current);  // delete last empty line
    }
}

void TextBuffer_SaveToFile(TextBuffer *tb, File *file) {
    TextBuffer_MergeGap(tb);
    Line *current = TextBuffer_GetFirstLine(tb);
    while (current) {
        File_WriteLine(file, &current->text);
        current = current->next;
    }
}
