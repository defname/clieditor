#include "textlayout.h"
#include "common/logging.h"

#include <stdlib.h>


void VisualLine_Init(VisualLine *vl, Line *src, int offset) {
    vl->src = src;
    vl->offset = offset;
}

void VisualLine_Deinit(VisualLine *vl) {
    vl->src = NULL;
    vl->offset = 0;
}


void TextLayout_Init(TextLayout *tl, const TextBuffer *tb, int display_width, int display_height) {
    tl->width = display_width;
    tl->height = display_height;
    tl->tb = tb;
    tl->dirty = true;

    VisualLine_Init(&tl->first_line, tb->current_line, 0);

    tl->cache = NULL;
    tl->cache_capacity = 0;
}

void TextLayout_Deinit(TextLayout *tl) {
    if (tl->cache) {
        free(tl->cache);
    }
    tl->cache = NULL;
    tl->cache_capacity = 0;
    VisualLine_Deinit(&tl->first_line);
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

void TextLayout_SetFirstLine(TextLayout *tl, Line *line, int offset) {
    tl->first_line.src = line;
    tl->first_line.offset = offset;
    tl->dirty = true;
}

bool TextLayout_ScrollUp(TextLayout *tl) {
    if (tl->first_line.offset >= tl->width) {
        tl->first_line.offset -= tl->width;
        tl->dirty = true;
        return true;
    }
    else if (tl->first_line.src->prev) {
        Line *new_src = tl->first_line.src->prev;
        int len = new_src->text.length;
        int w = tl->width;

        // Calculate the offset of the last visual line of the previous logical line.
        int new_offset = (len / w) * w;
        // If the line length is a multiple of the width and the line is not empty,
        // the calculated offset is out of bounds. We need to go to the start of the
        // *actual* last visual line.
        if (len > 0 && len % w == 0) {
            new_offset -= w;
        }
        tl->first_line.src = new_src;
        tl->first_line.offset = new_offset;
        tl->dirty = true;
        return true;
    }
    return false;
}


bool TextLayout_ScrollDown(TextLayout *tl) {
    if (tl->dirty) {
        TextLayout_Recalc(tl, 0);
    }
    // check if the end of the document is already reached.
    for (int i=0; i<tl->height; i++) {
        if (tl->cache[i].src == NULL) {
            return false;
        }
    }
    VisualLine *first_line = &tl->first_line;
    int length = first_line->src->text.length;
    if (first_line->src == tl->tb->current_line) {
        length += tl->tb->gap.position + tl->tb->gap.text.length - tl->tb->gap.overlap;
        tl->dirty = true;
        return true;
    }
    if (first_line->offset + tl->width < length) {
        first_line->offset += tl->width;
        tl->dirty = true;
        return true;
    }
    else if (first_line->src->next) {
        first_line->src = first_line->src->next;
        first_line->offset = 0;
        tl->dirty = true;
        return true;
    }
    return false;
}

static void increase_capacity(TextLayout *tl) {
    if (!tl) {
        return;
    }
    tl->cache_capacity = tl->height * 2;
    VisualLine *new_cache = realloc(tl->cache, sizeof(VisualLine) * tl->cache_capacity);
    if (!new_cache) {
        logFatal("Cannot allocate memory for VisualLine cache.");
    }
    tl->cache = new_cache;
    // initialize new
    for (size_t i=0; i<tl->cache_capacity; i++) {
        VisualLine_Init(&tl->cache[i], NULL, 0);
    }
}

void TextLayout_Recalc(TextLayout *tl, int start_y) {
    if (!tl || !tl->dirty || !tl->first_line.src || tl->width == 0 || tl->height == 0 || !tl->tb) {
        return;
    }

    if (tl->cache_capacity < (size_t)tl->height) {
        increase_capacity(tl);
    }

    if (start_y < 0) {
        start_y = 0;
    }
    if (start_y >= tl->height) {
        tl->dirty = false;
        return;
    }
    if (start_y == 0) {
        tl->cache[0] = tl->first_line;
        start_y = 1;
    }

    VisualLine *last = &tl->cache[start_y-1];
    int y = start_y;
    while (y < tl->height) {
        int last_length = last->src->text.length;
        // calc last length if last->src is the gap line
        if (last->src == tl->tb->current_line) {  // it's the gap line
            last_length += tl->tb->gap.position + tl->tb->gap.text.length - tl->tb->gap.overlap;
        }

        VisualLine *current = &tl->cache[y];
        // if there is length of last->src left for current
        if (last->offset + tl->width < last_length) {
            current->src = last->src;
            current->offset = last->offset + tl->width;
        }
        else if (last->src->next) {  // the end of the document isn't reached
            current->src = last->src->next;
            current->offset = 0;
        } else {  // reset rest of cache
            for (size_t i=y; i<tl->cache_capacity; i++) {
                VisualLine_Init(&tl->cache[i], NULL, 0);
            }
            break;
        }
        last = current;
        y++;
    }
    tl->dirty = false;
}

VisualLine *TextLayout_GetVisualLine(TextLayout *tl, int y) {
    if (!tl || !tl->cache || y < 0 || y >= tl->height) {
        return NULL;
    }
    if (tl->dirty) {
        TextLayout_Recalc(tl, 0);
    }
    if (tl->cache[y].src == NULL) {
        return NULL;
    }
    return &tl->cache[y];
}

int TextLayout_GetVisualLineLength(TextLayout *tl, int y) {
    VisualLine *line = TextLayout_GetVisualLine(tl, y);
    if (!line) {
        return -1;
    }
    if (line->offset + tl->width < (int)line->src->text.length) {
        return tl->width;
    }
    else {
        return line->src->text.length - line->offset;
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
    if (!tl) {
        return 0;
    }
    if (tl->dirty) {
        TextLayout_Recalc(tl, 0);
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
        TextLayout_Recalc(tl, 0);
    }

    Line *cursor_line = tl->tb->current_line;
    int cursor_pos_in_line = get_cursor_position_in_line(tl->tb);

    // Quick check: Is the cursor's source line completely above the visible area?
    if (cursor_line->position < tl->first_line.src->position) {
        return -1;
    }
    // If the cursor is in the same line as the first visible line, but in a wrapped
    // section that is scrolled off-screen above.
    if (cursor_line == tl->first_line.src && cursor_pos_in_line < tl->first_line.offset) {
        return -1;
    }

    // Iterate through the visible lines to find the cursor
    for (int y = 0; y < tl->height; ++y) {
        VisualLine *vl = &tl->cache[y];
        if (!vl->src) break; // End of document reached

        if (vl->src == cursor_line) {
            if (cursor_pos_in_line >= vl->offset && cursor_pos_in_line < vl->offset + tl->width) {
                return y; // Found: Cursor is on screen at visual line y
            }
        }
    }

    // If the loop finishes without finding the cursor, it must be below the screen.
    return tl->height;
}
