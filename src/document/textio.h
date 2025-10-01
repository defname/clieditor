#ifndef TEXTIO_H
#define TEXTIO_H

#include "textbuffer.h"
#include "io/file.h"


void TB_LoadFromFile(TextBuffer *tb, File *file);
void TB_SaveToFile(TextBuffer *tb, File *file);

#endif