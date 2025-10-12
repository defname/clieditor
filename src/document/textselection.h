#ifndef TEXTSELECTION_H
#define TEXTSELECTION_H

#include "textbuffer.h"
#include "common/utf8string.h"


typedef struct _TextSelection {
    Line *start;
    int start_idx;
    Line *end;
    int end_idx;
} TextSelection;

void TextSelection_Init(TextSelection *ts);
void TextSelection_Deinit(TextSelection *ts);

void TextSelection_Select(TextSelection *ts, Line *line, int idx);
void TextSelection_Abort(TextSelection *ts);

bool TextSelection_IsSelected(TextSelection *ts, Line *line, int idx);



bool TextSelection_Started(TextSelection *ts);

void TextSelection_Begin(TextSelection *ts, Line *line, int idx);
void TextSelection_End(TextSelection *ts, Line *line, int idx);

TextSelection TextSelection_Ordered(const TextSelection *ts);

void TextSelection_Extract(TextSelection *ts, UTF8String *text);
void TextSelection_Delete(TextSelection *ts, TextBuffer *tb);

#endif