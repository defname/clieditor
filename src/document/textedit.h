#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "textbuffer.h"
#include "textlayout.h"

typedef struct _TextEdit {
    TextBuffer *tb;
    TextLayout *tl;
} TextEdit;

void TextEdit_Init(TextEdit *te, TextBuffer *tb, TextLayout *tls);
void TextEdit_Deinit(TextEdit *te);

// --- Cursor movement ---
void TextEdit_MoveLeft(TextEdit *te);
void TextEdit_MoveRight(TextEdit *te);
void TextEdit_MoveUp(TextEdit *te);
void TextEdit_MoveDown(TextEdit *te);
/*
// --- Editing ---
void TextEdit_InsertChar(TextEdit *te, char c);
void TextEdit_DeleteChar(TextEdit *te);      // delete at cursor
void TextEdit_Backspace(TextEdit *te);       // delete before cursor
void TextEdit_Newline(TextEdit *te);

// --- Optional convenience ---
void TextEdit_InsertString(TextEdit *te, const char *text);
*/
#endif