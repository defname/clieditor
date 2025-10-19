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
#include "cell.h"


void Cell_Init(Cell *cell) {
    cell->cp = ' ';
    cell->style = (Style){ .fg = 0, .bg = 0, .attributes = STYLE_NONE };
    cell->changed = true;
}

bool Cell_Equal(const Cell *a, const Cell *b) {
    return a->cp == b->cp
        && a->style.fg == b->style.fg
        && a->style.bg == b->style.bg
        && a->style.attributes == b->style.attributes;
}