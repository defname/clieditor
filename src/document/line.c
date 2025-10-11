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

#include "line.h"

#include "common/logging.h"

Line *Line_Create() {
    Line *new_line = malloc(sizeof(Line));
    if (!new_line) {
        logFatal("Cannot allocate memory for TextBuffer.");
    }
    UTF8String_Init(&new_line->text);
    new_line->prev = NULL;
    new_line->next = NULL;
    new_line->position = 0;
    return new_line;
}

void Line_Destroy(Line *line) {
    if (!line) {
        return;
    }
    UTF8String_Deinit(&line->text);
    free(line);
}

static void rebuild_positions(Line *line) {
    if (!line) {
        logWarn("Invalid parameters for rebuild_positions.");
        return;
    }
    // find first line
    Line *first = line;
    while (first->prev) {
        first = first->prev;
    }
    // rebuild positions
    int position = 0;
    while (first) {
        first->position = position;
        position += LINE_POSITION_STEP;
        first = first->next;
    }
}

void Line_InsertBefore(Line *line, Line *new_line) {
    if (line->prev) {
        Line_InsertAfter(line->prev, new_line);
    }
    else {
        new_line->prev = NULL;
        new_line->next = line;
        line->prev = new_line;
        new_line->position = line->position - LINE_POSITION_STEP;
    }
}

void Line_InsertAfter(Line *line, Line *new_line) {
    if (!line || !new_line) {
        logWarn("Invalid parameters for Line_InsertAfter.");
        return;
    }
    Line *third = line->next;
    line->next = new_line;
    new_line->prev = line;
    new_line->next = third;
    if (third) {
        third->prev = new_line;
        int diff = third->position - line->position;
        if (diff < 2) {
            rebuild_positions(line);
            diff = third->position - line->position;
        }
        new_line->position = line->position + diff / 2;
    }
    else {
        new_line->position = line->position + LINE_POSITION_STEP;
    }
}

void Line_Delete(Line *line) {
    if (!line) {
        return;
    }
    if (line->prev) {
        line->prev->next = line->next;
    }
    if (line->next) {
        line->next->prev = line->prev;
    }
    Line_Destroy(line);
}