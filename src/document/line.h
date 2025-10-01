#ifndef LINE_H
#define LINE_H

#include "common/utf8string.h"

typedef struct _Line {
    UTF8String text;

    struct _Line *prev;
    struct _Line *next;
} Line;

Line *Line_Create();
void Line_Destroy(Line *l);
void Line_InsertAfter(Line *line, Line *new_line);
void Line_Delete(Line *line);

#endif