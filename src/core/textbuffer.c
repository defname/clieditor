#include "textbuffer.h"

#include "utils/logging.h"

Line *Line_Create() {
    Line *new_line = malloc(sizeof(Line));
    if (!new_line) {
        logFatal("Cannot allocate memory for TextBuffer.");
    }
    UTF8String_Init(&new_line->text);
    new_line->prev = NULL;
    new_line->next = NULL;
    return new_line;
}

void Line_Free(Line *line) {
    if (!line) {
        return;
    }
    UTF8String_Deinit(&line->text);
    free(line);
}

void Gap_Init(Gap *gap) {
    UTF8String_Init(&gap->text);
    gap->overlap = 0;
}

void Gap_Deinit(Gap *gap) {
    UTF8String_Deinit(&gap->text);
}

void TB_Init(TextBuffer *tb) {
    tb->cursor_pos = 0;
    tb->current_line = Line_Create();
    Gap_Init(&tb->gap);
}

void TB_Deinit(TextBuffer *tb) {
    Line *start = tb->current_line;
    while (start->prev) {
        start = start->prev;
    }
    while (start) {
        Line *tmp = start->next;
        Line_Free(start);
        start = tmp;
    }
    tb->current_line = NULL;
    Gap_Deinit(&tb->gap);
}

void TB_TextAroundGap(const TextBuffer *tb, UTF8String *before, UTF8String *after) {
    UTF8String_Split(&tb->current_line->text, before, after, tb->cursor_pos);
    UTF8String_Shorten(before, tb->cursor_pos - tb->gap.overlap);
}

void TB_MergeGap(TextBuffer *tb) {
    UTF8String *line = &tb->current_line->text;
    // split the line at cursor position
    UTF8String a, b;
    UTF8String_Init(&a);
    UTF8String_Init(&b);
    UTF8String_Split(line, &a, &b, tb->cursor_pos);
    // shrink the original line (it's now the part before the gap)
    UTF8String_Shorten(line, tb->cursor_pos - tb->gap.overlap);
    // concat parts
    UTF8String_Concat(line, &tb->gap.text);
    UTF8String_Concat(line, &b);
    // free parts
    UTF8String_Deinit(&a);
    UTF8String_Deinit(&b);
    // set the cursor position to end of the former gap
    tb->cursor_pos += tb->gap.text.length - tb->gap.overlap;
    // reset the gap
    tb->gap.text.length = 0;
    tb->gap.overlap = 0;
}

void TB_MoveCursor(TextBuffer *tb, int dx) {
    TB_MergeGap(tb);
    long new_pos = (long)tb->cursor_pos + dx;
    if (new_pos < 0) {
        tb->cursor_pos = 0;
        return;
    }
    if ((size_t)new_pos >= tb->current_line->text.length) {
        tb->cursor_pos = tb->current_line->text.length;
        return;
    }
    tb->cursor_pos = (size_t)new_pos;
}

void TB_ChangeLine(TextBuffer *tb, int ty) {
    TB_MergeGap(tb);

    if (ty > 0) {
        if (!tb->current_line->next) {
            return;
        }
        tb->current_line = tb->current_line->next;
    }
    else if (ty < 0) {
        if (!tb->current_line->prev) {
            return;
        }
        tb->current_line = tb->current_line->prev;
    }
    // fix cursor position if out-of-range
    if (tb->cursor_pos > tb->current_line->text.length) {
        tb->cursor_pos = tb->current_line->text.length;
    }

#if 0
    if (ty > 0) {
        for (int i=0; i<ty; i++) {
            if (!tb->current_line->next) {
                return;
            }
            tb->current_line = tb->current_line->next;
        }
    }
    else {
        for (int i=0; i>ty; i--) {
            if (!tb->current_line->prev) {
                return;
            }
            tb->current_line = tb->current_line->prev;
        }
    }
#endif
}

void TB_InsertLineAfter(TextBuffer *tb) {
    Line *new_line = Line_Create();
    Line *next = tb->current_line->next;
    new_line->next = next;
    new_line->prev = tb->current_line;
    tb->current_line->next = new_line;
    if (next) {
        next->prev = new_line;
    }
}

void TB_DeleteCurrentLine(TextBuffer *tb) {
    Line *prev = tb->current_line->prev;
    Line *next = tb->current_line->next;
    if (prev == NULL) {
        return;
    }
    prev->next = next;
    if (next != NULL) {
        next->prev = prev;
    }
    Line *old_current = tb->current_line;
    tb->current_line = prev;
    Line_Free(old_current);
}

void TB_InsertChar(TextBuffer *tb, UTF8Char ch) {
    UTF8String_AddChar(&tb->gap.text, ch);
}

void TB_Backspace(TextBuffer *tb) {
    if (tb->cursor_pos == 0 && tb->gap.text.length == 0) {
        TB_MergeGap(tb);
        if (tb->current_line->prev == NULL) {
            return;
        }
        int cursor_pos = tb->current_line->prev->text.length;
        UTF8String_Concat(&tb->current_line->prev->text, &tb->current_line->text);
        TB_DeleteCurrentLine(tb);
        tb->cursor_pos = cursor_pos;
        return;
    }
    if (tb->gap.text.length == 0) {
        if (tb->cursor_pos <= tb->gap.overlap) {
            TB_MergeGap(tb);
            TB_Backspace(tb);
            return;
        }
        tb->gap.overlap++;
        return;
    }
    // gap length > 0
    tb->gap.text.length--;
}

void TB_Enter(TextBuffer *tb) {
    TB_MergeGap(tb);
    UTF8String before, after;
    UTF8String_Init(&before);
    UTF8String_Init(&after);
    UTF8String_Split(&tb->current_line->text, &before, &after, tb->cursor_pos);
    tb->current_line->text.length = tb->cursor_pos;
    TB_InsertLineAfter(tb);
    Line *next = tb->current_line->next;
    UTF8String_Copy(&next->text, &after);
    tb->current_line = next;
    tb->cursor_pos = 0;


    UTF8String_Deinit(&before);
    UTF8String_Deinit(&after);
}