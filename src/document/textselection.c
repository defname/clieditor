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
    if (!ts || !ts->start || !ts->end || !line || idx < 0 || idx > (int)line->text.length) {
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

void TextSelection_Extract(TextSelection *ts, UTF8String *text) {
    if (!ts || !ts->start || !ts->end) {
        return;
    }

    Line *start, *end;
    int start_idx, end_idx;
    ordered(ts, &start, &start_idx, &end, &end_idx);

    UTF8String dummy;
    UTF8String_Init(&dummy);

    Line *current = start;

    if (current == end) {
        UTF8String_SubString(&current->text, text, start_idx, end_idx - start_idx);
    }
    else {
        UTF8String_SubString(&current->text, text, start_idx, current->text.length - start_idx);
        current = current->next;
        while (current != end) {
            UTF8String_AddChar(text, utf8_newline);
            UTF8String_Concat(text, &current->text);
            current = current->next;
        }
        UTF8String_AddChar(text, utf8_newline);
        UTF8String_SubString(&current->text, &dummy, 0, end_idx);
        UTF8String_Concat(text, &dummy);
    }

    UTF8String_Deinit(&dummy);
}
