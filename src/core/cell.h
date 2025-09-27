#ifndef CELL_H
#define CELL_H

#include <stdint.h>
#include <stdbool.h>
#include "utf8.h"

#define STYLE_NONE      0
#define STYLE_BOLD      1
#define STYLE_UNDERLINE 2

typedef struct {
    uint8_t fg; // foreground color (ANSI-Code 0-255)
    uint8_t bg; // background color (ANSI-Code 0-255)
    uint16_t attributes; // Bit-Flags für fett, unterstrichen etc.
} Style;

typedef struct _Cell {
    UTF8Char ch;
    Style style;
    bool changed;  // marks if the Cell has changed since the last rendering
} Cell;

void Cell_Init(Cell *cell);

bool Cell_Equal(const Cell *a, const Cell *b);

#endif