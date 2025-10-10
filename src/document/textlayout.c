#include "textlayout.h"
#include "common/logging.h"

#include <stdlib.h>


void VisualLine_Init(VisualLine *vl) {
    vl->src = NULL;
    vl->offset = 0;
    vl->length = 0;
    vl->width = 0;
}

void VisualLine_Deinit(VisualLine *vl) {
    VisualLine_Init(vl);
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
    tl->dirty = true;
}

void TextLayout_SetFirstLine(TextLayout *tl, Line *line, int fist_visual_line_idx) {
    tl->first_line = line;
    tl->first_visual_line_idx = fist_visual_line_idx;
    tl->dirty = true;
}

static int calc_tab_width(int x_pos, int tabstop) {
    return (x_pos + 1) % tabstop;
}

int TextLayout_CalcTabWidth(TextLayout *tl, int x_pos) {
    return calc_tab_width(x_pos, tl->width);
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
    // initialize new
    for (size_t i=0; i<tl->cache_capacity; i++) {
        VisualLine_Init(&tl->cache[i]);
    }
}

// vl: target, line: src, offset: position in line to start, with: display width
static void calc_visual_line(VisualLine *vl, Line *line, int offset, int width, int tabstop) {
    vl->src = line;
    vl->offset = offset;
    int w = 0;
    size_t i = offset;
    while (1) {
        if (i >= line->text.length) {
            break;
        }
        UTF8Char ch = line->text.chars[i];
        int char_w;
        if (UTF8_EqualToChar(ch, '\t')) {
            char_w = calc_tab_width(w, tabstop);
        }
        else {
            char_w = UTF8_GetWidth(ch);
        }
        if (w + char_w > width) {
            break;
        }
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
            calc_visual_line(line, tl->first_line, 0, tl->width, tl->tabstop);
            continue;
        }
        VisualLine *prev = &tl->cache[cache_idx - 1];
        size_t new_offset = prev->offset + prev->length;
        if (new_offset >= prev->src->text.length) {  // prev line consumed src completely
            Line *next_src = prev->src->next;
            if (!next_src) {  // end of document reached
                break;
            }
            // continue with next src line
            calc_visual_line(line, next_src, 0, tl->width, tl->tabstop);
            continue;
        }
        // continue with last src
        calc_visual_line(line, prev->src, prev->offset + prev->length, tl->width, tl->tabstop);
        continue;
    }
     // clear rest of cache if there is no next line
    for ( ; cache_idx<(int)tl->cache_capacity; cache_idx++) {
        VisualLine_Init(&tl->cache[cache_idx]);
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

int TextLayout_GetVisualLineLength(TextLayout *tl, int y) {
    VisualLine *line = TextLayout_GetVisualLine(tl, y);
    if (!line) {
        return -1;
    }
    int length = line->src->text.length;
    if (line->src == tl->tb->current_line) {
        length += tl->tb->gap.text.length - tl->tb->gap.overlap;
    }

    if (line->offset + tl->width < length) {
        return tl->width;
    }
    else {
        int remaining = length - line->offset;
        return (remaining > 0) ? remaining : 0;
    }
}

int get_cursor_position_in_line(const TextBuffer *tb) {
    if (!tb) {
        return 0;
    }
    int pos = tb->gap.position + tb->gap.text.length - tb->gap.overlap;
    if (pos < 0) {
        logDebug("This should not happen at all... Check gap manipulating functions.");
        return 0;
    }
    return pos;
}

int TextLayout_GetCursorX(TextLayout *tl) {
    // TODO!!!
    if (!tl) {
        return 0;
    }
    if (tl->dirty) {
        TextLayout_Recalc(tl, -tl->first_visual_line_idx);
    }
    int position_in_line = get_cursor_position_in_line(tl->tb);
    if (position_in_line < 0) {
        logFatal("Not sure if this can happen");
    }
    int cursor_x = position_in_line % tl->width;
    return cursor_x;
}

int TextLayout_GetCursorY(TextLayout *tl) {
    if (!tl || !tl->tb) {
        return -1; // Not on screen if layout/buffer is invalid
    }
    if (tl->dirty) {
        TextLayout_Recalc(tl, -tl->first_visual_line_idx);
    }

    Line *cursor_line = tl->tb->current_line;
    int cursor_pos_in_line = get_cursor_position_in_line(tl->tb);

    // Quick check: Is the cursor's source line completely above the visible area?
    if (cursor_line->position < tl->first_line->position) {
        return -1;
    }
    // If the cursor is in the same line as the first visible line, but in a wrapped
    // section that is scrolled off-screen above.
    if (cursor_line == tl->first_line && cursor_pos_in_line < tl->cache[tl->first_visual_line_idx].offset) {
        return -1;
    }

    // Iterate through the visible lines to find the cursor
    for (int y = tl->first_visual_line_idx; y < tl->first_visual_line_idx + tl->height; y++) {
        VisualLine *vl = &tl->cache[y];
        if (!vl->src) break; // End of document reached

        if (vl->src == cursor_line) {
            // The cursor is on this visual line if its position is within the character range [offset, offset + length).
            // The special case is when the cursor is at the very end of the line (offset + length),
            // it still belongs to the current visual line.
            bool is_at_end_of_line = (cursor_pos_in_line == vl->offset + vl->length);
            bool is_within_line = (cursor_pos_in_line >= vl->offset && cursor_pos_in_line < vl->offset + vl->length);

            if (is_within_line || is_at_end_of_line) {
                return y - tl->first_visual_line_idx;
            }
        }
    }

    // If the loop finishes without finding the cursor, it must be below the screen.
    return tl->height;
}
