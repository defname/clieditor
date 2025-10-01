#include "textcursor.h"

#include "common/logging.h"


size_t TB_CursorPos(const TextBuffer *tb) {
    return tb->gap.position - tb->gap.overlap + tb->gap.text.length;
}

bool TB_AtFirstLine(const TextBuffer *tb) {
    return tb->current_line->prev == NULL;
}

bool TB_AtLastLine(const TextBuffer *tb) {
    return tb->current_line->next == NULL;
}

bool TB_AtHome(const TextBuffer *tb) {
    return tb->gap.position == 0;
}

bool TB_AtEnd(const TextBuffer *tb) {
    return tb->gap.position == tb->current_line->text.length;
}

UTF8Char TB_CharUnderCursor(const TextBuffer *tb) {
    return tb->current_line->text.chars[tb->gap.position];
}

void TB_MoveCursor(TextBuffer *tb, int dx) {
    TB_MergeGap(tb);
    long new_pos = (long)tb->gap.position + dx;
    if (new_pos < 0) {
        if (!tb->current_line->prev) {
            tb->gap.position = 0;
            return;
        }
        TB_ChangeLine(tb, -1);
        tb->gap.position = tb->current_line->text.length;
        return;
    }
    if ((size_t)new_pos >= tb->current_line->text.length) {
        if (!tb->current_line->next) {
            tb->gap.position = tb->current_line->text.length;
            return;
        }
        TB_ChangeLine(tb, 1);
        tb->gap.position = 0;
        return;
    }
    tb->gap.position = (size_t)new_pos;
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
    if (tb->gap.position > tb->current_line->text.length) {
        tb->gap.position = tb->current_line->text.length;
    }
}

void TB_End(TextBuffer *tb) {
    TB_MergeGap(tb);
    tb->gap.position = tb->current_line->text.length;
}

void TB_Home(TextBuffer *tb) {
    TB_MergeGap(tb);
    tb->gap.position = 0;
}