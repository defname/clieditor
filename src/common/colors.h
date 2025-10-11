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
#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    COLOR_MODE_256,
    COLOR_MODE_16
} ColorMode;


typedef enum {
    COLOR_FG,
    COLOR_BG,
    COLOR_HIGHLIGHT_FG,
    COLOR_HIGHLIGHT_BG,
    COLOR_PRIMARY_FG,
    COLOR_PRIMARY_BG,
    COLOR_SECONDARY_FG,
    COLOR_SECONDARY_BG,
    COLOR_SUCCESS,
    COLOR_WARNING,
    COLOR_ERROR,
} ColorId;

typedef struct _Color {
    uint8_t index_16;
    uint8_t index_256;
} Color;

void Color_SetMode(ColorMode mode);
ColorMode Color_GetMode();

bool Color_Equal(const Color *a, const Color *b);

Color Color_ById(ColorId id);
uint8_t Color_GetCode(Color color);
uint8_t Color_GetCodeById(ColorId id);

#endif