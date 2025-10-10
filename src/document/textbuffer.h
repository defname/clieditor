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

#include "line.h"

typedef struct _Gap {
    UTF8String text;
    size_t position;
    size_t overlap;
} Gap;

void Gap_Init(Gap *gap);
void Gap_Deinit(Gap *gap);

typedef struct _TextBuffer {
    Line *current_line;
    Gap gap;

    size_t line_count;
} TextBuffer;

void TextBuffer_Init(TextBuffer *tb);
void TextBuffer_Deinit(TextBuffer *tb);
void TextBuffer_ReInit(TextBuffer *tb);

void TextBuffer_TextAroundGap(const TextBuffer *tb, UTF8String *before, UTF8String *after);
void TextBuffer_MergeGap(TextBuffer *tb);

void TextBuffer_InsertLineAfterCurrent(TextBuffer *tb, Line *new_line);
void TextBuffer_InsertLineAtTop(TextBuffer *tb, Line *new_line);
void TextBuffer_InsertLineAtBottom(TextBuffer *tb, Line *new_line);
bool TextBuffer_DeleteLine(TextBuffer *tb, Line *line);

Line *TextBuffer_GetFirstLine(const TextBuffer *tb);
Line *TextBuffer_GetLastLine(const TextBuffer *tb);

#endif