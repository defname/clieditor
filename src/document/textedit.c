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
        tb->gap.position = tb->current_line->text.length;
    }
    else {
        tb->gap.position = 0;
    }
}

void TextEdit_MoveRight(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextBuffer_MergeGap(tb);

    if (tb->gap.position < tb->current_line->text.length) {
        tb->gap.position++;
        return;
    }
    else if (tb->current_line->next) {
        tb->current_line = tb->current_line->next;
        tb->gap.position = 0;
    }
    else {
        tb->gap.position = tb->current_line->text.length;
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
    if (tb->gap.position > tb->current_line->text.length) {
        tb->gap.position = tb->current_line->text.length;
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
            tb->gap.position = tb->current_line->text.length;
            return;
        }
        cursor.y--;
    }
    VisualLine *line_below = TextLayout_GetVisualLine(tl, cursor.y + 1);
    if (!line_below) {
        tb->gap.position = tb->current_line->text.length;
        return;
    }
    tb->current_line = line_below->src;
    tb->gap.position = line_below->offset + VisualLine_GetOffsetForX(line_below, cursor.x);

    // fix position out of bounds
    if (tb->gap.position > tb->current_line->text.length) {
        tb->gap.position = tb->current_line->text.length;
    }
}

// --- Editing ---
void TextEdit_InsertChar(TextEdit *te, UTF8Char ch) {
    UTF8String_AddChar(&te->tb->gap.text, ch);
    te->tl->dirty = true;
}

void TextEdit_DeleteChar(TextEdit *te) {
    TextBuffer *tb = te->tb;
    te->tl->dirty = true;
    TextBuffer_MergeGap(tb);
    if (tb->gap.position - tb->gap.overlap + tb->gap.text.length < tb->current_line->text.length) {
        tb->gap.position++;
        tb->gap.overlap++;
        return;
    }
    // cursor is at last position of current_line
    if (!tb->current_line->next) {
        return;  // no next line... nothing to do
    }
    UTF8String_Concat(&tb->current_line->text, &tb->current_line->next->text);
    TextBuffer_DeleteLine(tb, tb->current_line->next);
}

void TextEdit_Backspace(TextEdit *te) {
    TextBuffer *tb = te->tb;
    te->tl->dirty = true;
    if (tb->gap.text.length > 0) {
        UTF8String_Shorten(&tb->gap.text, tb->gap.text.length - 1);
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
    tb->gap.position = tb->current_line->text.length;
    UTF8String_Concat(&tb->current_line->text, &tb->current_line->next->text);
    TextBuffer_DeleteLine(tb, tb->current_line->next);
}

void TextEdit_Newline(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextBuffer_MergeGap(tb);
    Line *new_line = Line_Create();
    UTF8String dummy;
    UTF8String_Init(&dummy);
    UTF8String_Split(&tb->current_line->text, &dummy, &new_line->text, tb->gap.position);
    UTF8String_Deinit(&dummy);
    UTF8String_Shorten(&tb->current_line->text, tb->gap.position);
    TextBuffer_InsertLineAfterCurrent(tb, new_line);
    tb->current_line = new_line;
    tb->gap.position = 0;
    te->tl->dirty = true;
}

// --- Optional convenience ---
void TextEdit_InsertString(TextEdit *te, UTF8String *string) {
    UTF8String_Concat(&te->tb->gap.text, string);
    te->tl->dirty = true;
}

