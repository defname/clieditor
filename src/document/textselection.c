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
#include "textselection.h"

void TextSelection_Init(TextSelection *ts) {
    ts->start = NULL;
    ts->start_idx = 0;
    ts->end = NULL;
    ts->end_idx = 0;
}

void TextSelection_Deinit(TextSelection *ts) {
    ts->start = NULL;
    ts->start_idx = 0;
    ts->end = NULL;
    ts->end_idx = 0;
}

void TextSelection_Select(TextSelection *ts, Line *line, int idx) {
    if (!TextSelection_Started(ts)) {
        TextSelection_Begin(ts, line, idx);
    }
    else {
        TextSelection_End(ts, line, idx);
    }
}

void TextSelection_Abort(TextSelection *ts) {
    ts->start = NULL;
    ts->start_idx = 0;
    ts->end = NULL;
    ts->end_idx = 0;
}

static void ordered(const TextSelection *ts, Line **start, int *start_idx, Line **end, int *end_idx) {
    if (ts->start->position < ts->end->position || (ts->start == ts->end && ts->start_idx < ts->end_idx)) {
        *start = ts->start;
        *end = ts->end;
        *start_idx = ts->start_idx;
        *end_idx = ts->end_idx;
    }
    else {
        *start = ts->end;
        *end = ts->start;
        *start_idx = ts->end_idx;
        *end_idx = ts->start_idx;
    }
}

bool TextSelection_IsSelected(TextSelection *ts, Line *line, int idx) {
    if (!ts || !ts->start || !ts->end || !line || idx < 0 || idx > (int)String_Length(&line->text)) {
        return false;
    }
    Line *start, *end;
    int start_idx, end_idx;
    ordered(ts, &start, &start_idx, &end, &end_idx);

    bool check_begin = line->position > start->position || (line == start && idx >= start_idx);
    bool check_end = line->position < end->position || (line == end && idx < end_idx);

    return check_begin && check_end;
}

bool TextSelection_Started(TextSelection *ts) {
    return ts->start != NULL;
}

void TextSelection_Begin(TextSelection *ts, Line *line, int idx) {
    ts->start = line;
    ts->start_idx = idx;
    ts->end = line;
    ts->end_idx = idx;
}

void TextSelection_End(TextSelection *ts, Line *line, int idx) {
    ts->end = line;
    ts->end_idx = idx;
}

TextSelection TextSelection_Ordered(const TextSelection *ts) {
    TextSelection o;
    ordered(ts, &o.start, &o.start_idx, &o.end, &o.end_idx);
    return o;
}

String TextSelection_Extract(TextSelection *ts) {
    if (!ts || !ts->start || !ts->end) {
        return String_Empty();
    }

    Line *start, *end;
    int start_idx, end_idx;
    ordered(ts, &start, &start_idx, &end, &end_idx);

    String out;

    Line *current = start;

    if (current == end) {
        out = String_Substring(&current->text, start_idx, end_idx - start_idx);
    }
    else {
        out = String_Substring(&current->text, start_idx, String_Length(&current->text) - start_idx);
        current = current->next;
        while (current != end) {
            String_AddChar(&out, "\n");
            String_Append(&out, &current->text);
            current = current->next;
        }
        String_AddChar(&out, "\n");
        String dummy = String_Substring(&current->text, 0, end_idx);
        String_Append(&out, &dummy);
        String_Deinit(&dummy);
    }

    return out;
}

void TextSelection_Delete(TextSelection *ts, TextBuffer *tb) {
    if (!ts || !ts->start || !ts->end) {
        return;
    }
    TextSelection sel = TextSelection_Ordered(ts);

    String end = String_Substring(&sel.end->text, sel.end_idx, String_Length(&sel.end->text) - sel.end_idx);
    
    if (sel.start == sel.end) {
        String_Shorten(&sel.start->text, sel.start_idx);
        String_Append(&sel.start->text, &end);
    }
    else {
        tb->current_line = sel.start;  // cannot delete current line, so place it here
        Line *current = sel.end;
        current = current->prev;
        TextBuffer_DeleteLine(tb, current->next);
        while (current != sel.start) {
            current = current->prev;
            TextBuffer_DeleteLine(tb, current->next);
        }
        String_Shorten(&sel.start->text, sel.start_idx);
        String_Append(&sel.start->text, &end);
    }

    tb->gap.position = sel.start_idx;

    String_Deinit(&end);
}