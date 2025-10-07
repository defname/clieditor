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

    int cursor_x = TextLayout_GetCursorX(tl);
    int cursor_y = TextLayout_GetCursorY(tl);

    if (cursor_y == 0) {
        // try to scroll up 
        if (!TextLayout_ScrollUp(tl)) {
            // jump to the beginning of the line if scrolling was not possible (begin of document reached)
            tb->gap.position = 0;
            return;
        }
        cursor_y = 1;
    }
    VisualLine *line_above = TextLayout_GetVisualLine(tl, cursor_y - 1);
    tb->current_line = line_above->src;
    tb->gap.position = line_above->offset + cursor_x;
    
    // fix position out of bounds
    if (tb->gap.position > tb->current_line->text.length) {
        tb->gap.position = tb->current_line->text.length;
    }
}

void TextEdit_MoveDown(TextEdit *te) {
    TextBuffer *tb = te->tb;
    TextLayout *tl = te->tl;
    TextBuffer_MergeGap(tb);

    int cursor_x = TextLayout_GetCursorX(tl);
    int cursor_y = TextLayout_GetCursorY(tl);

    if (cursor_y == tl->height-1) {
        // try to scroll down 
        if (!TextLayout_ScrollDown(tl)) {
            // jump to the end of the line if scrolling was not possible (end of document reached)
            tb->gap.position = tb->current_line->text.length;
            return;
        }
        cursor_y--;
    }
    VisualLine *line_below = TextLayout_GetVisualLine(tl, cursor_y + 1);
    if (!line_below) {
        tb->gap.position = tb->current_line->text.length;
        return;
    }
    tb->current_line = line_below->src;
    tb->gap.position = line_below->offset + cursor_x;

    // fix position out of bounds
    if (tb->gap.position > tb->current_line->text.length) {
        tb->gap.position = tb->current_line->text.length;
    }
}

// --- Editing ---
void TextEdit_InsertChar(TextEdit *te, char c);
void TextEdit_DeleteChar(TextEdit *te);      // delete at cursor
void TextEdit_Backspace(TextEdit *te);       // delete befoTextBuffer_MergeGap(te->tb);re cursor
void TextEdit_Newline(TextEdit *te);

// --- Optional convenience ---
void TextEdit_InsertString(TextEdit *te, const char *text);

