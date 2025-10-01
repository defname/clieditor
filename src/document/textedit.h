#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "textbuffer.h"

Line *TB_InsertLineAfter(TextBuffer *tb);

void TB_InsertChar(TextBuffer *tb, UTF8Char ch);

void TB_Backspace(TextBuffer *tb);
void TB_Delete(TextBuffer *tb);
void TB_Enter(TextBuffer *tb);

#endif