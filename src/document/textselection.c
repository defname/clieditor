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

static void sort_start_end(TextSelection *ts) {
    if (ts->start->position < ts->end->position || (ts->start == ts->end && ts->start_idx < ts->end_idx)) {
        return;
    }
    Line *tmp;
    int tmp_idx;

    tmp = ts->end;
    ts->end = ts->start;
    ts->start = tmp;

    tmp_idx = ts->end_idx;
    ts->end_idx = ts->start_idx;
    ts->start_idx = tmp_idx;
}

bool TextSelection_IsSelected(TextSelection *ts, Line *line, int idx) {
    if (!ts || !ts->start || !ts->end || !line || idx < 0 || idx > (int)line->text.length) {
        return false;
    }
    
    sort_start_end(ts);

    bool check_begin = line->position > ts->start->position || (line == ts->start && idx >= ts->start_idx);
    bool check_end = line->position < ts->end->position || (line == ts->end && idx < ts->end_idx);

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

    sort_start_end(ts);

    UTF8String dummy;
    UTF8String_Init(&dummy);

    Line *current = ts->start;

    if (current == ts->end) {
        UTF8String_SubString(&current->text, text, ts->start_idx, ts->end_idx - ts->start_idx);
    }
    else {
        UTF8String_SubString(&current->text, text, ts->start_idx, current->text.length - ts->start_idx);
        current = current->next;
        while (current != ts->end) {
            UTF8String_AddChar(text, utf8_newline);
            UTF8String_Concat(text, &current->text);
            current = current->next;
        }
        UTF8String_AddChar(text, utf8_newline);
        UTF8String_SubString(&current->text, &dummy, 0, ts->end_idx);
        UTF8String_Concat(text, &dummy);
    }

    UTF8String_Deinit(&dummy);
}
