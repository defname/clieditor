#include "textio.h"

#include "textedit.h"

void TextBuffer_LoadFromFile(TextBuffer *tb, File *file) {
    TextBuffer_ReInit(tb);
    Line *first = tb->current_line;
    Line *current = tb->current_line;
    UTF8String *line;
    while ((line = File_ReadLine(file)) != NULL) {
        UTF8String_Copy(&current->text, line);
        UTF8String_Destroy(line);
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
