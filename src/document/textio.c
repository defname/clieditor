#include "textio.h"

#include "textedit.h"

void TB_LoadFromFile(TextBuffer *tb, File *file) {
    TB_ReInit(tb);
    Line *current = tb->current_line;
    UTF8String *line;
    while ((line = File_ReadLine(file)) != NULL) {
        UTF8String_Copy(&current->text, line);
        UTF8String_Destroy(line);
        current = TB_InsertLineAfter(tb);
    }
}
