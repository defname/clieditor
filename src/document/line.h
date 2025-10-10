#ifndef LINE_H
#define LINE_H

#include "common/utf8string.h"

#define LINE_POSITION_STEP 100

typedef struct _Line {
    UTF8String text;

    int position;

    struct _Line *prev;
    struct _Line *next;
} Line;

Line *Line_Create();
void Line_Destroy(Line *l);
void Line_InsertBefore(Line *line, Line *new_line);
void Line_InsertAfter(Line *line, Line *new_line);
void Line_Delete(Line *line);

#endif