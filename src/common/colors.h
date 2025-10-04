#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>

typedef enum {
    COLOR_MODE_256,
    COLOR_MODE_16
} ColorMode;


typedef enum {
    COLOR_FG,
    COLOR_BG,
    COLOR_HIGHLIGHT_FG,
    COLOR_HIGHLIGHT_BG,
    COLOR_HIGHLIGHT,
    COLOR_PRIMARY,
    COLOR_SECONDARY,
    COLOR_SUCCESS,
    COLOR_WARNING,
    COLOR_ERROR,
} ColorId;

typedef struct _Color {
    unsigned char index_16;
    unsigned char index_256;
} Color;

void Color_SetMode(ColorMode mode);
ColorMode Color_GetMode();

bool Color_Equal(const Color *a, const Color *b);

Color Color_ById(ColorId id);
unsigned char Color_GetCode(Color color);
unsigned char Color_GetCodeById(ColorId id);

#endif