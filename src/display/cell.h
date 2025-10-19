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
#ifndef CELL_H
#define CELL_H

#include <stdint.h>
#include <stdbool.h>
#include "display/style.h"

typedef struct _Cell {
    uint32_t cp;
    Style style;
    bool changed;  // marks if the Cell has changed since the last rendering
} Cell;

void Cell_Init(Cell *cell);

bool Cell_Equal(const Cell *a, const Cell *b);

#endif