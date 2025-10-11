#ifndef TEXTIO_H
#define TEXTIO_H

#include "textbuffer.h"
#include "io/file.h"


void TextBuffer_LoadFromFile(TextBuffer *tb, File *file);
void TextBuffer_SaveToFile(TextBuffer *tb, File *file);

#endif