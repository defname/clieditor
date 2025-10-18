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

#ifndef UTF8_HELPER_H
#define UTF8_HELPER_H

// used by utf8_get_codepoint if ch is not a valid UTF8 character
#define INVALID_CODEPOINT 0xFFFFFFFF

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


size_t utf8_get_char_length(unsigned char c);
bool utf8_is_continuation_byte(unsigned char c);
size_t utf8_strlen(const char *str);
uint32_t utf8_get_codepoint(const char *ch);
int utf8_get_width(const char *ch);

#endif