#include "textio.h"

#include "textedit.h"

void TB_LoadFromFile(TextBuffer *tb, File *file) {
    TB_ReInit(tb);
    Line *first_line = tb->current_line;
    Line *current = tb->current_line;
    UTF8String *line;
    while ((line = File_ReadLine(file)) != NULL) {
        UTF8String_Copy(&current->text, line);
        UTF8String_Destroy(line);
        current = TB_InsertLineAfter(tb);
        tb->current_line = current;
    }
    // current is now a last empty line which was not in the document
    // so delete it
    if (tb->line_count > 1) {
        Line_Delete(current);
        tb->line_count--;
    }
    tb->current_line = first_line;
}

void TB_SaveToFile(const TextBuffer *tb, File *file) {
    Line *current = TB_GetFirstLine(tb);
    while (current) {
        File_WriteLine(file, &current->text);
        current = current->next;
    }
}
