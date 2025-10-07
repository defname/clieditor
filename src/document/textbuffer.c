#include "document/textbuffer.h"

#include "common/logging.h"

void Gap_Init(Gap *gap) {
    UTF8String_Init(&gap->text);
    gap->overlap = 0;
}

void Gap_Deinit(Gap *gap) {
    UTF8String_Deinit(&gap->text);
}

void TextBuffer_Init(TextBuffer *tb) {
    tb->gap.position = 0;
    tb->line_pos = 0;
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

void TextBuffer_TextAroundGap(const TextBuffer *tb, UTF8String *before, UTF8String *after) {
    UTF8String_Split(&tb->current_line->text, before, after, tb->gap.position);
    UTF8String_Shorten(before, tb->gap.position - tb->gap.overlap);
}

void TextBuffer_MergeGap(TextBuffer *tb) {
    UTF8String *line = &tb->current_line->text;
    // split the line at cursor position
    UTF8String a, b;
    UTF8String_Init(&a);
    UTF8String_Init(&b);
    UTF8String_Split(line, &a, &b, tb->gap.position);
    // shrink the original line (it's now the part before the gap)
    UTF8String_Shorten(line, tb->gap.position - tb->gap.overlap);
    // concat parts
    UTF8String_Concat(line, &tb->gap.text);
    UTF8String_Concat(line, &b);
    // free parts
    UTF8String_Deinit(&a);
    UTF8String_Deinit(&b);
    // set the cursor position to end of the former gap
    tb->gap.position += (long)tb->gap.text.length - (long)tb->gap.overlap;
    // reset the gap
    tb->gap.text.length = 0;
    tb->gap.overlap = 0;
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
