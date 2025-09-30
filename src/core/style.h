#ifndef STYLE_H
#define STYLE_H

#include <stdint.h>
#include <stdbool.h>

#define STYLE_NONE      0
#define STYLE_BOLD      1
#define STYLE_UNDERLINE 2

typedef struct {
    uint8_t fg; // foreground color (ANSI-Code 0-255)
    uint8_t bg; // background color (ANSI-Code 0-255)
    uint16_t attributes; // Bit-Flags für fett, unterstrichen etc.
} Style;

bool Style_Equal(const Style *a, const Style *b);

#endif