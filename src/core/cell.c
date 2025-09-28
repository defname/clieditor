#include "cell.h"


void Cell_Init(Cell *cell) {
    cell->ch = utf8_space;
    cell->style = (Style){ .fg = 0, .bg = 0, .attributes = STYLE_NONE };
    cell->changed = true;
}

bool Cell_Equal(const Cell *a, const Cell *b) {
    return UTF8_Equal(a->ch, b->ch)
        && a->style.fg == b->style.fg
        && a->style.bg == b->style.bg
        && a->style.attributes == b->style.attributes;
}