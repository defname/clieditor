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
#include "textlayout.h"

#include <stdlib.h>
#include <string.h>

#include "common/logging.h"
#include "common/utf8_helper.h"


void VisualLine_Init(VisualLine *vl, int screen_width) {
    if (screen_width <= 0) {
        logFatal("Invalid screen_width for VisualLine.");
    }
    vl->char_x = malloc(sizeof(int) * (size_t)screen_width);
    if (!vl->char_x) {
        logFatal("Cannot allocate memory for char_x in VisualLine.");
    }
    VisualLine_Reset(vl, screen_width);
}

void VisualLine_Deinit(VisualLine *vl) {
    if (vl->char_x) {
        free(vl->char_x);
    }
    vl->src = NULL;
    vl->gap = NULL;
    vl->offset = 0;
    vl->length = 0;
    vl->width = 0;
    vl->char_x = NULL;
}

void VisualLine_Reset(VisualLine *vl, int screen_width) {
    vl->src = NULL;
    vl->gap = NULL;
    vl->offset = 0;
    vl->length = 0;
    vl->width = 0;
    if (!vl->char_x) {
        logFatal("No memory allocated for char_x in VisualLine.");
    }
    memset(vl->char_x, 0, sizeof(int) * screen_width);
}

void VisualLine_Resize(VisualLine *vl, int screen_width) {
    vl->char_x = realloc(vl->char_x, sizeof(int) * (size_t)screen_width);
    VisualLine_Reset(vl, screen_width);
}

int VisualLine_GetOffsetForX(const VisualLine *vl, int x) {
    for (int i = 0; i < vl->length; i++) {
        if (vl->char_x[i] >= x) {
            return i;
        }
    }
    return vl->length;
}


int VisualLine_GetLength(const VisualLine *vl) {
    if (!vl) {
        return 0;
    }
    return vl->length;
}

int VisualLine_GetWidth(const VisualLine *vl) {
    if (!vl) {
        return 0;
    }
    return vl->width;
}

int VisualLine_GetCharX(const VisualLine *vl, int idx) {
    if (!vl || idx < 0 || idx >= vl->length) {
        logFatal("Invalid arguments to VisualLine_GetCharX");
        return 0;
    }
    return vl->char_x[idx];
}

const char *VisualLine_GetChar(const VisualLine *vl, int i) {
    if (!vl || i < 0) {
        logFatal("Invalid arguments to VisualLine_GetChar");
        return NULL;
    }
    int idx = i + vl->offset;
    if (!vl->gap) {  // there is no gap on this line
        if (idx < (int)String_Length(&vl->src->text)) {
            return String_GetChar(&vl->src->text, idx);
        }
        logFatal("Index out of range in VisualLine_GetChar");
        return NULL;
    }
    // the index is before the gap
    if (idx < (int)vl->gap->position - (int)vl->gap->overlap) {
        return String_GetChar(&vl->src->text, idx);
    }
    // index is in the gap
    if (idx < (int)vl->gap->position - (int)vl->gap->overlap + (int)String_Length(&vl->gap->text)) {
        return String_GetChar((String*)&vl->gap->text, idx - vl->gap->position + vl->gap->overlap);
    }
    // index is after the gap but within the total length of the line
    if (i < (int)vl->length) {
        return String_GetChar(&vl->src->text, idx - String_Length(&vl->gap->text) + vl->gap->overlap);
    }
    logFatal("Index out of range in VisualLine_GetChar");
    return NULL;
    
}


void TextLayout_Init(TextLayout *tl, const TextBuffer *tb, int display_width, int display_height) {
    tl->width = display_width;
    tl->height = display_height;
    tl->tabstop = 4;
    tl->tb = tb;
    tl->dirty = true;

    tl->first_line = tb->current_line;
    tl->first_visual_line_idx = 0;

    tl->cache = NULL;
    tl->cache_capacity = 0;
}

void TextLayout_Deinit(TextLayout *tl) {
    if (tl->cache) {
        for (size_t i = 0; i < tl->cache_capacity; i++) {
            VisualLine_Deinit(&tl->cache[i]);
        }
        free(tl->cache);
    }
    tl->cache = NULL;
    tl->cache_capacity = 0;
    tl->width = 0;
    tl->height = 0;
    tl->tb = NULL;
    tl->dirty = false;
}

void TextLayout_SetDimensions(TextLayout *tl, int display_width, int display_height) {
    tl->width = display_width;
    tl->height = display_height;
    for (size_t i=0; i<tl->cache_capacity; i++) {
        VisualLine_Resize(&tl->cache[i], display_width);
    }
    tl->dirty = true;
}

void TextLayout_SetFirstLine(TextLayout *tl, Line *line, int fist_visual_line_idx) {
    tl->first_line = line;
    tl->first_visual_line_idx = fist_visual_line_idx;
    tl->dirty = true;
}

static int calc_tab_width(int x_pos, int tabstop) {
    return tabstop - (x_pos % tabstop);
}

int TextLayout_CalcTabWidth(TextLayout *tl, int x_pos) {
    return calc_tab_width(x_pos, tl->tabstop);
}

static int first_line_height(const TextLayout *tl) {
    if (!tl || tl->cache_capacity == 0) {
        return 0;
    }
    Line *first_line_src = tl->first_line;
    int h = 0;
    VisualLine *line = line = &tl->cache[0];;
    while (line->src == first_line_src) {
        line = &tl->cache[++h];
    }
    return h;
}

bool TextLayout_AtTop(TextLayout *tl) {
    if (!tl || tl->cache_capacity == 0) {
        return true;
    }
    return tl->first_visual_line_idx == 0 && !tl->first_line->prev;
}

bool TextLayout_AtBottom(TextLayout *tl) {
    if (!tl) {
        return true;
    }
    if (tl->dirty) {
        TextLayout_Recalc(tl, -tl->first_visual_line_idx);
    }
    if (tl->cache_capacity < (size_t)tl->height) {
        return true;
    }
    return tl->cache[tl->first_visual_line_idx + tl->height - 1].src == NULL;
}

bool TextLayout_ScrollUp(TextLayout *tl) {
    if (tl->first_visual_line_idx > 0) {
        if (tl->dirty) {
            TextLayout_Recalc(tl, -tl->first_visual_line_idx);
        }
        tl->first_visual_line_idx--;
        return true;
    }
    Line *first_line_src = tl->first_line;
    if (!first_line_src->prev) {  // already at the beginning of the document
        return false;
    }
    // Set previous line as first line and recalc
    Line *new_first_line = first_line_src->prev;
    TextLayout_SetFirstLine(tl, new_first_line, 0);
    TextLayout_Recalc(tl, 0);

    // if the previous line will split into more than one visual lines
    // the first_visual_line_idx need to be adjusted
    tl->first_visual_line_idx = first_line_height(tl) - 1;
    TextLayout_Recalc(tl, 0);
    return true;
}


bool TextLayout_ScrollDown(TextLayout *tl) {
    if (tl->dirty) {
        TextLayout_Recalc(tl, -tl->first_visual_line_idx);
    }
    // check if the end of the document is already reached.
    if (tl->cache[tl->first_visual_line_idx + tl->height - 1].src == NULL) {
        return false;
    }

    if (tl->cache_capacity <= (size_t)tl->first_visual_line_idx + 1) {
        return false;
    }

    // if there is still a wrap of the first line to scroll to
    if (tl->first_line == tl->cache[tl->first_visual_line_idx + 1].src) {
        tl->first_visual_line_idx++;
        TextLayout_Recalc(tl, tl->height-2);
        return true;
    }

    if (!tl->first_line->next) {
        return false;
    }

    TextLayout_SetFirstLine(tl, tl->first_line->next, 0);
    TextLayout_Recalc(tl, 0);
    return true;
}

static void increase_cache_capacity(TextLayout *tl) {
    if (!tl) {
        return;
    }
    size_t old_capacity = tl->cache_capacity;
    if (tl->cache_capacity <= (size_t)tl->height) {
        tl->cache_capacity = tl->height * 2;
    }
    else {
        tl->cache_capacity *= 2;
    }
    VisualLine *new_cache = realloc(tl->cache, sizeof(VisualLine) * tl->cache_capacity);
    if (!new_cache) {
        logFatal("Cannot allocate memory for VisualLine cache.");
    }
    tl->cache = new_cache;
    // initialize only the new part of the cache
    for (size_t i = old_capacity; i < tl->cache_capacity; i++) {
        VisualLine_Init(&tl->cache[i], tl->width);
    }
}

// vl: target, line: src, offset: position in line to start, with: display width
static void calc_visual_line(VisualLine *vl, Line *line, int offset, TextLayout *tl) {
    int width = tl->width;
    int tabstop = tl->tabstop;
    const TextBuffer *tb = tl->tb;

    vl->src = line;
    vl->offset = offset;
    int w = 0;
    int i = offset;

    const Gap *gap = NULL;
    vl->gap = NULL;
    int text_length = String_Length(&line->text);
    if (line == tb->current_line) {
        gap = &tb->gap;
        text_length += String_Length(&gap->text) - gap->overlap;
        vl->gap = gap;
    }

    while (1) {
        if (i >= text_length) {
            break;
        }
        const char *ch;
        if (gap) {
            if (i < (int)gap->position - (int)gap->overlap) {
                ch = String_GetChar(&line->text, i);
            }
            else if (i < (int)gap->position - (int)gap->overlap + (int)String_Length(&gap->text)) {
                ch = String_GetChar((String*)&gap->text, i - gap->position + gap->overlap);
            }
            else {
                ch = String_GetChar(&line->text, i - String_Length(&gap->text) + gap->overlap);
            }
        }
        else {
            ch = String_GetChar(&line->text, i);
        }

        
        int char_w;
        uint32_t cp = utf8_to_codepoint(ch);
        if (cp == '\t') {
            char_w = calc_tab_width(w, tabstop);
        }
        else {
            char_w = utf8_calc_width(cp);
        }
        if (w + char_w > width) {
            break;
        }
        vl->char_x[i-offset] = w;
        w += char_w;
        i++;
    }
    vl->length = i - offset;
    vl->width = w;
}

void TextLayout_Recalc(TextLayout *tl, int start_y) {
    if (!tl || !tl->first_line || tl->width == 0 || tl->height == 0 || !tl->tb) {
        return;
    }
    int cache_idx = start_y + tl->first_visual_line_idx;
    for (; cache_idx<tl->height + tl->first_visual_line_idx; cache_idx++) {
        if ((int)tl->cache_capacity <= cache_idx) {
            increase_cache_capacity(tl);
        }
        VisualLine *line = &tl->cache[cache_idx];
        if (cache_idx == 0) {
            calc_visual_line(line, tl->first_line, 0, tl);
            continue;
        }
        VisualLine *prev = &tl->cache[cache_idx - 1];
        size_t new_offset = prev->offset + prev->length;
        int text_length = String_Length(&prev->src->text);
        if (prev->src == tl->tb->current_line) {
            text_length += String_Length(&tl->tb->gap.text) - tl->tb->gap.overlap;
        }
        if ((int)new_offset >= text_length) {  // prev line consumed src completely
            Line *next_src = prev->src->next;
            if (!next_src) {  // end of document reached
                break;
            }
            // continue with next src line
            calc_visual_line(line, next_src, 0, tl);
            continue;
        }
        // continue with last src
        calc_visual_line(line, prev->src, prev->offset + prev->length, tl);
        continue;
    }
     // clear rest of cache if there is no next line
    for ( ; cache_idx<(int)tl->cache_capacity; cache_idx++) {
        VisualLine_Reset(&tl->cache[cache_idx], tl->width);
    }
    tl->dirty = false;
}

VisualLine *TextLayout_GetVisualLine(TextLayout *tl, int y) {
    if (tl->dirty) {
        TextLayout_Recalc(tl, -tl->first_visual_line_idx);
    }
    // Check after Recalc, because Recalc allocates the cache.
    if (!tl || !tl->cache || y < 0 || y >= tl->height || y + tl->first_visual_line_idx >= (int)tl->cache_capacity) {
        return NULL;
    }
    if (tl->cache[y + tl->first_visual_line_idx].src == NULL) {
        return NULL;
    }
    return &tl->cache[y + tl->first_visual_line_idx];
}

int get_cursor_idx_in_line(const TextBuffer *tb) {
    if (!tb) {
        return 0;
    }
    int pos = tb->gap.position + String_Length(&tb->gap.text) - tb->gap.overlap;
    if (pos < 0) {
        logDebug("This should not happen at all... Check gap manipulating functions.");
        return 0;
    }
    return pos;
}


int TextLayout_GetCursorLayoutInfo(TextLayout *tl, CursorLayoutInfo *info) {
    if (!tl || !tl->tb) {
        return -1; // Not on screen if layout/buffer is invalid
    }
    if (tl->dirty) {
        TextLayout_Recalc(tl, -tl->first_visual_line_idx);
    }

    info->on_screen = -1;
    info->x = 0;
    info->y = 0;
    info->idx = 0;
    info->line = NULL;

    Line *cursor_line = tl->tb->current_line;
    int cursor_idx_in_line = get_cursor_idx_in_line(tl->tb);

    // Quick check: Is the cursor's source line completely above the visible area?
    if (cursor_line->position < tl->first_line->position) {
        return -1;
    }
    // If the cursor is in the same line as the first visible line, but in a wrapped
    // section that is scrolled off-screen above.
    if (cursor_line == tl->first_line && cursor_idx_in_line < tl->cache[tl->first_visual_line_idx].offset) {
        return -1;
    }

    // Iterate through the visible lines to find the cursor
    for (int cache_idx = tl->first_visual_line_idx; cache_idx < tl->first_visual_line_idx + tl->height; cache_idx++) {
        VisualLine *vl = &tl->cache[cache_idx];
        if (!vl->src) break; // End of document reached

        if (vl->src == cursor_line) {
            // if the line is the end of the line the cursor can be at the last position (behind the last char)
            int src_line_length = (int)String_Length(&vl->src->text) + (int)String_Length(&tl->tb->gap.text) - (int)tl->tb->gap.overlap;
            bool line_ends_here = (vl->offset + vl->length == src_line_length);
            bool is_at_end_of_line = (line_ends_here && cursor_idx_in_line == src_line_length);
            // otherwise (if the line is broken and the cursor is not at the total end of the line)
            // the cursor will go to the beginning of the next line when reaching behind the last char of a visual line
            bool is_within_visual_line = (cursor_idx_in_line >= vl->offset && cursor_idx_in_line < vl->offset + vl->length);

            if (is_within_visual_line || is_at_end_of_line) {

                info->on_screen = 0;
                info->idx = cursor_idx_in_line - vl->offset;
                info->x = info->idx < vl->length ? vl->char_x[info->idx] : vl->width;
                info->y = cache_idx - tl->first_visual_line_idx;
                info->line = vl;

                return 0;
            }
        }
    }

    // If the loop finishes without finding the cursor, it must be below the screen.
    info->on_screen = 1;
    return 1;
}
