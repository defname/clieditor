#include "textio.h"

#include "textedit.h"

void TextBuffer_LoadFromFile(TextBuffer *tb, File *file) {
    TextBuffer_ReInit(tb);
    Line *current = tb->current_line;
    UTF8String *line;
    while ((line = File_ReadLine(file)) != NULL) {
        UTF8String_Copy(&current->text, line);
        UTF8String_Destroy(line);
        Line *newline = Line_Create();
        Line_InsertAfter(current, newline);
        current = newline;
    }
    // current is now a last empty line which was not in the document
    // so delete it
    if (tb->line_count > 1) {
        Line_Delete(current);
        tb->line_count--;
    }
}

void TextBuffer_SaveToFile(const TextBuffer *tb, File *file) {
    Line *current = TextBuffer_GetFirstLine(tb);
    while (current) {
        File_WriteLine(file, &current->text);
        current = current->next;
    }
}
