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
