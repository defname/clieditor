#include "colors.h"

static ColorMode active_mode = COLOR_MODE_256;

static Color color_table[] = {
    [COLOR_FG] 			 = { 7, 252},
    [COLOR_BG] 			 = { 0, 236},
    [COLOR_HIGHLIGHT_FG] = {15, 255},
    [COLOR_HIGHLIGHT_BG] = { 8, 239},
    [COLOR_PRIMARY_FG]   = { 7, 252},
    [COLOR_PRIMARY_BG]   = {12,  33},
    [COLOR_SECONDARY_FG] = { 7, 252},
    [COLOR_SECONDARY_BG] = { 5, 127},
    [COLOR_SUCCESS]      = {10,  76},
    [COLOR_WARNING]      = { 1, 214},
    [COLOR_ERROR]        = { 9, 160},
};

void Color_SetMode(ColorMode mode) {
    active_mode = mode;
}

ColorMode Color_GetMode() {
    return active_mode;
}

bool Color_Equal(const Color *a, const Color *b) {
    return a->index_16 == b->index_16 && a->index_256 == b->index_256;
}

Color Color_ById(ColorId id) {
    return color_table[id];
}

unsigned char Color_GetCode(Color color) {
    return active_mode == COLOR_MODE_256 ? color.index_256 : color.index_16;
}

unsigned char Color_GetCodeById(ColorId id) {
    return Color_GetCode(Color_ById(id));
}