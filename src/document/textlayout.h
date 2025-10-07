#ifndef TEXTLAYOUT_H
#define TEXTLAYOUT_H

#include "textbuffer.h"

#include <stdbool.h>


typedef struct _VisualLine {
    Line *src;
    int offset;     //< where does the virtual line start. offset in characters
} VisualLine;

void VisualLine_Init(VisualLine *vl, Line *src, int offset);
void VisualLine_Deinit(VisualLine *vl);


typedef struct _TextLayout {
    const TextBuffer *tb;
    int width;
    int height;
    VisualLine first_line;

    VisualLine *cache;
    size_t cache_capacity;

    bool dirty;
} TextLayout;

void TextLayout_Init(TextLayout *tl, const TextBuffer *tb, int display_w, int display_h);
void TextLayout_Deinit(TextLayout *tl);

void TextLayout_SetDimensions(TextLayout *tl, int display_w, int display_h);
void TextLayout_SetFirstLine(TextLayout *tl, Line *line, int offset);

/**
 * @brief Recalc the TextLayout cache, starting at line start_y.
 * 
 * It is mandatory that if start_y > 0 the cache until start_y-1 is valid.
 * If just line y is edited it might be cheaper to call this function with start_y = y.
 */
void TextLayout_Recalc(TextLayout *tl, int start_y);

/**
 * @brief Return the visual line at y position.
 * 
 * If y is out of range or the document does not reach y return NULL.
 */
VisualLine *TextLayout_GetVisualLine(TextLayout *tl, int y);

/**
 * @brief Return the x position of the cursor with repect to gap properties
 */
int TextLayout_GetCursorX(TextLayout *tl);

/**
 * @brief Return the y position of the cursor on screen.
 * 
 * If the cursor is above the screen return -1 if the cursor is below screen return tl->height.
 */
int TextLayout_GetCursorY(TextLayout *tl);


#endif