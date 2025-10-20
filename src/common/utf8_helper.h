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

// used by utf8_to_codepoint if ch is not a valid UTF8 character
#define INVALID_CODEPOINT 0xFFFFFFFF

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Return the number of bytes used by the character that starts with c.
 */
size_t utf8_get_char_length(unsigned char c);

/**
 * @brief Return true if c is a valid continuation byte for an UTF-8 character.
 */
bool utf8_is_continuation_byte(unsigned char c);

/**
 * @brief Return the number of characters of an NULL-terminated UTF-8 string.
 */
size_t utf8_strlen(const char *str);

/**
 * @brief Count the number of characters only looking in bytes. If bytes == -1 it's counted up to '\0' (so it's utf8_strlen())
 */
size_t utf8_count_chars(const char *str, ssize_t bytes);

/**
 * @brief Calculate the UTF-8 codepoint of the first character in ch.
 */
uint32_t utf8_to_codepoint(const char *ch);

/**
 * @brief Write the UTF-8 codepoint as string to out and return the number of bytes used.
 */
size_t utf8_from_codepoint(uint32_t cp, char *out);

/**
 * @brief Return true if the UTF-8 codepoint is an ASCII character.
 */
bool utf8_is_ascii(uint32_t cp);

/**
 * @brief Calculate the display width of an UTF-8 codepoint.
 */
int utf8_calc_width(uint32_t cp);

#endif