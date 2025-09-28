#ifndef CELL_H
#define CELL_H

#include <stdint.h>
#include <stdbool.h>
#include "utf8.h"
#include "style.h"

typedef struct _Cell {
    UTF8Char ch;
    Style style;
    bool changed;  // marks if the Cell has changed since the last rendering
} Cell;

void Cell_Init(Cell *cell);

bool Cell_Equal(const Cell *a, const Cell *b);

#endif