#include "textedit.h"

Line *TB_InsertLineAfter(TextBuffer *tb) {
    Line *new_line = Line_Create();
    tb->line_count++;
    Line *next = tb->current_line->next;
    new_line->next = next;
    new_line->prev = tb->current_line;
    tb->current_line->next = new_line;
    if (next) {
        next->prev = new_line;
    }

    return new_line;
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
    Line_Destroy(old_current);
    tb->line_count--;
}

void TB_InsertChar(TextBuffer *tb, UTF8Char ch) {
    UTF8String_AddChar(&tb->gap.text, ch);
}

void TB_Backspace(TextBuffer *tb) {
    if (tb->gap.position == 0 && tb->gap.text.length == 0) {
        TB_MergeGap(tb);
        if (tb->current_line->prev == NULL) {
            return;
        }
        int cursor_pos = tb->current_line->prev->text.length;
        UTF8String_Concat(&tb->current_line->prev->text, &tb->current_line->text);
        TB_DeleteCurrentLine(tb);
        tb->gap.position = cursor_pos;
        return;
    }
    if (tb->gap.text.length == 0) {
        if (tb->gap.position <= tb->gap.overlap) {
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
    UTF8String_Split(&tb->current_line->text, &before, &after, tb->gap.position);
    tb->current_line->text.length = tb->gap.position;
    TB_InsertLineAfter(tb);
    Line *next = tb->current_line->next;
    UTF8String_Copy(&next->text, &after);
    tb->current_line = next;
    tb->gap.position = 0;


    UTF8String_Deinit(&before);
    UTF8String_Deinit(&after);
}
