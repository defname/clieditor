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
#include "document/textbuffer.h"

#include "common/logging.h"

void Gap_Init(Gap *gap) {
    String_Init(&gap->text);
    gap->overlap = 0;
}

void Gap_Deinit(Gap *gap) {
    String_Deinit(&gap->text);
}

void TextBuffer_Init(TextBuffer *tb) {
    tb->gap.position = 0;
    tb->current_line = Line_Create();
    tb->line_count = 1;
    Gap_Init(&tb->gap);
}

void TextBuffer_Deinit(TextBuffer *tb) {
    Line *start = tb->current_line;
    while (start->prev) {
        start = start->prev;
    }
    while (start) {
        Line *tmp = start->next;
        Line_Destroy(start);
        start = tmp;
    }
    tb->current_line = NULL;
    Gap_Deinit(&tb->gap);
}

void TextBuffer_ReInit(TextBuffer *tb) {
    TextBuffer_Deinit(tb);
    TextBuffer_Init(tb);
}

void TextBuffer_TextAroundGap(const TextBuffer *tb, StringView *before, StringView *after) {
    *before = String_Slice(&tb->current_line->text, 0, tb->gap.position - tb->gap.overlap);
    *after = String_Slice(&tb->current_line->text, tb->gap.position, String_Length(&tb->current_line->text));
}

void TextBuffer_MergeGap(TextBuffer *tb) {
    String *line = &tb->current_line->text;
    // text after cursor position
    String after = String_Substring(line, tb->gap.position, String_Length(line) - tb->gap.position);
    // shrink the original line (it's now the part before the gap)
    String_Shorten(line, tb->gap.position - tb->gap.overlap);
    // concat parts
    String_Append(line, &tb->gap.text);
    String_Append(line, &after);
    
    String_Deinit(&after);
    
    // set the cursor position to end of the former gap
    tb->gap.position += (long)String_Length(&tb->gap.text) - (long)tb->gap.overlap;
    // reset the gap
    String_Clear(&tb->gap.text);
    tb->gap.overlap = 0;
}

void TextBuffer_InsertLineAfterCurrent(TextBuffer *tb, Line *new_line) {
    Line_InsertAfter(tb->current_line, new_line);
    tb->line_count++;
}

void TextBuffer_InsertLineAtTop(TextBuffer *tb, Line *new_line) {
    Line *top = TextBuffer_GetFirstLine(tb);
    Line_InsertBefore(top, new_line);
    tb->line_count++;
}

void TextBuffer_InsertLineAtBottom(TextBuffer *tb, Line *new_line) {
    Line *bottom = TextBuffer_GetLastLine(tb);
    Line_InsertAfter(bottom, new_line);
    tb->line_count++;
}

bool TextBuffer_DeleteLine(TextBuffer *tb, Line *line) {
    if (!line || line == tb->current_line) {
        return false;
    }
    Line_Delete(line);
    tb->line_count--;
    return true;
}

Line *TextBuffer_GetFirstLine(const TextBuffer *tb) {
    Line *current = tb->current_line;
    while (current->prev) {
        current = current->prev;
    }
    return current;
}

Line *TextBuffer_GetLastLine(const TextBuffer *tb) {
    Line *current = tb->current_line;
    while (current->next) {
        current = current->next;
    }
    return current;
}
