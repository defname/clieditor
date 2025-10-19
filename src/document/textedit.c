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
#include "textedit.h"
#include "common/logging.h"
#include "common/utf8_helper.h"

void TextEdit_Init(TextEdit *te, TextBuffer *tb, TextLayout *tl) {
    if (!te || !tb || !tl || tb != tl->tb) {
        logError("Invalid arguments to initialize TextEdit().");
        return;
    }
    te->tb = tb;
    te->tl = tl;
}
void TextEdit_Deinit(TextEdit *te) {
    te->tb = NULL;
    te->tl = NULL;
}

// --- Cursor movement ---
void TextEdit_MoveLeft(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextBuffer_MergeGap(tb);
    
    if (tb->gap.position > 0) {
        tb->gap.position--;
        return;
    }
    if (tb->current_line->prev) {
        tb->current_line = tb->current_line->prev;
        tb->gap.position = String_Length(&tb->current_line->text);
    }
    else {
        tb->gap.position = 0;
    }
}

void TextEdit_MoveRight(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextBuffer_MergeGap(tb);

    if (tb->gap.position < String_Length(&tb->current_line->text)) {
        tb->gap.position++;
        return;
    }
    else if (tb->current_line->next) {
        tb->current_line = tb->current_line->next;
        tb->gap.position = 0;
    }
    else {
        tb->gap.position = String_Length(&tb->current_line->text);
    }
}

void TextEdit_MoveUp(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextLayout *tl = te->tl;
    TextBuffer_MergeGap(tb);

    CursorLayoutInfo cursor;
    int on_screen = TextLayout_GetCursorLayoutInfo(tl, &cursor);

    if (on_screen < 0) {
        // TODO
        logFatal("Debug msg...");
    }

    if (cursor.y == 0) {
        // try to scroll up 
        if (!TextLayout_ScrollUp(tl)) {
            // jump to the beginning of the line if scrolling was not possible (begin of document reached)
            tb->gap.position = 0;
            return;
        }
        cursor.y = 1;
    }
    VisualLine *line_above = TextLayout_GetVisualLine(tl, cursor.y - 1);
    tb->current_line = line_above->src;
    tb->gap.position = line_above->offset + VisualLine_GetOffsetForX(line_above, cursor.x);
    
    // fix position out of bounds
    if (tb->gap.position > String_Length(&tb->current_line->text)) {
        tb->gap.position = String_Length(&tb->current_line->text);
    }
}

void TextEdit_MoveDown(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextLayout *tl = te->tl;
    TextBuffer_MergeGap(tb);

    CursorLayoutInfo cursor;
   TextLayout_GetCursorLayoutInfo(tl, &cursor);

    if (cursor.y == tl->height-1) {
        // try to scroll down 
        if (!TextLayout_ScrollDown(tl)) {
            // jump to the end of the line if scrolling was not possible (end of document reached)
            tb->gap.position = String_Length(&tb->current_line->text);
            return;
        }
        cursor.y--;
    }
    VisualLine *line_below = TextLayout_GetVisualLine(tl, cursor.y + 1);
    if (!line_below) {
        tb->gap.position = String_Length(&tb->current_line->text);
        return;
    }
    tb->current_line = line_below->src;
    tb->gap.position = line_below->offset + VisualLine_GetOffsetForX(line_below, cursor.x);

    // fix position out of bounds
    if (tb->gap.position > String_Length(&tb->current_line->text)) {
        tb->gap.position = String_Length(&tb->current_line->text);
    }
}

// --- Editing ---
void TextEdit_InsertChar(TextEdit *te, uint32_t cp) {
    char buf[5];
    size_t len = utf8_from_codepoint(cp, buf);
    if (len == 0) {
        return;
    }
    buf[len] = '\0';
    String_AddChar(&te->tb->gap.text, buf);
    te->tl->dirty = true;
}

void TextEdit_DeleteChar(TextEdit *te) {
    TextBuffer *tb = te->tb;
    te->tl->dirty = true;
    TextBuffer_MergeGap(tb);
    if (tb->gap.position - tb->gap.overlap + String_Length(&tb->gap.text) < String_Length(&tb->current_line->text)) {
        tb->gap.position++;
        tb->gap.overlap++;
        return;
    }
    // cursor is at last position of current_line
    if (!tb->current_line->next) {
        return;  // no next line... nothing to do
    }
    String_Append(&tb->current_line->text, &tb->current_line->next->text);
    TextBuffer_DeleteLine(tb, tb->current_line->next);
}

void TextEdit_Backspace(TextEdit *te) {
    TextBuffer *tb = te->tb;
    te->tl->dirty = true;
    if (String_Length(&tb->gap.text) > 0) {
        String_Shorten(&tb->gap.text, String_Length(&tb->gap.text) - 1);
        return;
    }
    if (tb->gap.position - tb->gap.overlap > 0) {
        tb->gap.overlap++;
        return;
    }
    // gap length is 0 and gap overlap is at max
    TextBuffer_MergeGap(tb);
    // so the newline needs to be deleted
    // this means to concat the current line to the previous line
    if (!tb->current_line->prev) {
        return;  // no prev line... nothing to do
    }
    tb->current_line = tb->current_line->prev;
    tb->gap.position = String_Length(&tb->current_line->text);
    String_Append(&tb->current_line->text, &tb->current_line->next->text);
    TextBuffer_DeleteLine(tb, tb->current_line->next);
}

void TextEdit_Newline(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextBuffer_MergeGap(tb);
    Line *new_line = Line_Create();

    // get text after cursor
    size_t after_cursor_len = String_Length(&tb->current_line->text) - tb->gap.position;
    String after_cursor = String_Substring(&tb->current_line->text, tb->gap.position, after_cursor_len);
    // shorten current line
    String_Shorten(&tb->current_line->text, tb->gap.position);
    // transfer ownership of after_cursor to new_line
    String_Take(&new_line->text, &after_cursor);
    // insert new line
    TextBuffer_InsertLineAfterCurrent(tb, new_line);
    tb->current_line = new_line;
    tb->gap.position = 0;
    te->tl->dirty = true;
}

// --- Optional convenience ---
void TextEdit_InsertString(TextEdit *te, const String *string) {
    String_Append(&te->tb->gap.text, string);
    te->tl->dirty = true;
}

