/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * This module is responsible for calculating which lines are visible, where they are wrapped
 * and where the cursor actually is on screen.
 */
#ifndef TEXTLAYOUT_H
#define TEXTLAYOUT_H

#include "textbuffer.h"

#include <stdbool.h>

/**
 * @brief Represents a visual line on screen.
 */
typedef struct _VisualLine {
    Line *src;
    const Gap *gap;       //< pointer to the gap if it's relevant for this visual line
    int offset;     //< where does the virtual line start. offset in characters
    int length;     //< how many characters are used
    int width;      //< how many cells are needed to render this line
    int *char_x;    //< x positions of the characters of this visual line
} VisualLine;

void VisualLine_Init(VisualLine *vl, int screen_width);
void VisualLine_Deinit(VisualLine *vl);
void VisualLine_Reset(VisualLine *vl, int screen_width);
void VisualLine_Resize(VisualLine *vl, int screen_width);

int VisualLine_GetOffsetForX(const VisualLine *vl, int x);
int VisualLine_GetLength(const VisualLine *vl);
int VisualLine_GetWidth(const VisualLine *vl);
int VisualLine_GetCharX(const VisualLine *vl, int idx);
const char *VisualLine_GetChar(const VisualLine *vl, int idx);


/**
 * @brief Holds all relevant information about the cursor posiition.
 */
typedef struct _CursorLayoutInfo {
    int on_screen;      //< -1 if it's above screen, 1 if it's below screen and 0 if it's on screen
    int x;              //< x position on the screen (with respect to wide characters and tab, might be equal to screen width if a line is exactly as wide)
    int y;              //< y position on screen
    int idx;            //< idx in the VisualLine
    VisualLine *line;   //< VisualLine the cursor is in
} CursorLayoutInfo;


/**
 * @brief Hold width and height and a cache of calculated VisualLines.
 */
typedef struct _TextLayout {
    const TextBuffer *tb;  //< const since this module does only calculates information and doesn't change anything

    // This properties can and should be manipulated by the user
    int width;
    int height;
    int tabstop;

    Line *first_line;           //< not necessarily completely visible
    int first_visual_line_idx;  //< cache idx;

    // This is created automatically
    VisualLine *cache;
    size_t cache_capacity;

    // Should be set if tb is manipulated
    bool dirty;
} TextLayout;

void TextLayout_Init(TextLayout *tl, const TextBuffer *tb, int display_w, int display_h);
void TextLayout_Deinit(TextLayout *tl);

void TextLayout_SetDimensions(TextLayout *tl, int display_w, int display_h);
void TextLayout_SetFirstLine(TextLayout *tl, Line *line, int offset);

/**
 * @brief Return the width of a tab character at position x
 */
int TextLayout_CalcTabWidth(TextLayout *tl, int x_pos);

/**
 * @brief Beginning of document reached
 */
bool TextLayout_AtTop(TextLayout *tl);

/**
 * @brief End of document reached
 */
bool TextLayout_AtBottom(TextLayout *tl);

/**
 * @brief Scroll one line up. Return true if actually scrolled.
 */
bool TextLayout_ScrollUp(TextLayout *tl);

/**
 * @brief Scroll one line down. Make sure TextBuffer_MergeGap() was called before.
 */
bool TextLayout_ScrollDown(TextLayout *tl);

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
 * @brief Return the length of the line at y position. (-1 if not on screen or out of document)
 */
int TextLayout_GetVisualLineLength(TextLayout *tl, int y);

/**
 * @brief Place all relevant information about the cursor position in info.
 * 
 * @returns -1 if the cursor is above the screen, 1 if it's below the screen and 0 if it's on screen.
 */
int TextLayout_GetCursorLayoutInfo(TextLayout *tl, CursorLayoutInfo *info);


#endif