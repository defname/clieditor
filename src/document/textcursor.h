#ifndef TEXTCURSOR_H
#define TEXTCURSOR_H

#include "textbuffer.h"


size_t TB_CursorPos(const TextBuffer *tb);


bool TB_AtFirstLine(const TextBuffer *tb);
bool TB_AtLastLine(const TextBuffer *tb);
bool TB_AtHome(const TextBuffer *tb);
bool TB_AtEnd(const TextBuffer *tb);

UTF8Char TB_CharUnderCursor(const TextBuffer *tb);

void TB_MoveCursor(TextBuffer *tb, int dx);
void TB_ChangeLine(TextBuffer *tb, int dy);

void TB_End(TextBuffer *tb);
void TB_Home(TextBuffer *tb);

#endif