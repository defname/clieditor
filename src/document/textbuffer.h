/**
 * textbuffer.h
 * Provides the core functionality of the editor.
 * The text is split in lines which are connected as a double-linked-list.
 * Text modification uses a gap, that is at the position of the cursor.
 * If the cursor moves the gap is merged into the text.
 * 
 * It looks a bit like this:
 * 
 *   +-----------------------------------------------------------------+
 * +-> prev                                                            |
 * | +-----------------------------------------------------------------+
 * | 
 * | +-----------------------------------------------------------------+
 * +-| current line                                                    |-+
 *   +----------------^------------------------------------------------+ |
 *                    |                                                  |
 *   |< cursor pos   >|                                                  |
 *                    |                                                  |
 *    overlap |<     >|                                                  |
 *                    |                                                  |
 *            +---------------+                                          |
 *            | gap           |                                          |
 *            +---------------+                                          |
 *   +-----------------------------------------------------------------+ |
 *   | next                                                            <-+
 *   +-----------------------------------------------------------------+
 * 
 * if current line holds
 * "I like Terminals."
 * and the gap is at pos 2 with overlap 0 with text "really "
 * 
 * "I like terminals."
 *    ^
 *   "really "
 * 
 * after mergin it is
 * "I really like terminals".
 * 
 * With the same sentence and a gap at pos 6 with overlap 4 and the text "love"
 * 
 * "I like terminals."
 *       ^
 *   "love"
 * 
 * the result is
 * "I love terminals."
 */
#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include "common/utf8string.h"
#include "io/file.h"

typedef struct _Line {
    UTF8String text;

    struct _Line *prev;
    struct _Line *next;
} Line;

Line *Line_Create();
void LineFree(Line *l);

typedef struct _Gap {
    UTF8String text;
    size_t overlap;
} Gap;

void Gap_Init(Gap *gap);
void Gap_Deinit(Gap *gap);

typedef struct _TextBuffer {
    Line *current_line;
    size_t cursor_pos;
    Gap gap;
} TextBuffer;

void TB_Init(TextBuffer *tb);
void TB_Deinit(TextBuffer *tb);
void TB_ReInit(TextBuffer *tb);

void TB_LoadFromFile(TextBuffer *tb, File *file);
void TB_SaveToFile(TextBuffer *tb, File *file);

void TB_TextAroundGap(const TextBuffer *tb, UTF8String *before, UTF8String *after);
void TB_MergeGap(TextBuffer *tb);

void TB_MoveCursor(TextBuffer *tb, int dx);
void TB_ChangeLine(TextBuffer *tb, int dy);

Line *TB_InsertLineAfter(TextBuffer *tb);

void TB_InsertChar(TextBuffer *tb, UTF8Char ch);

void TB_Backspace(TextBuffer *tb);
void TB_Delete(TextBuffer *tb);
void TB_Enter(TextBuffer *tb);

#endif