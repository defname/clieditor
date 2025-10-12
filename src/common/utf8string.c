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
#include "common/utf8string.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "common/logging.h"


void UTF8String_Init(UTF8String *str) {
    str->capacity = 0;
    str->length = 0;
    str->chars = NULL;
    UTF8String_Resize(str, UTF8STRING_INITIAL_CAPACITY);
}

void UTF8String_Deinit(UTF8String *str) {
    if (str->chars != NULL) {
        free(str->chars);
    }
    str->capacity = 0;
    str->length = 0;
    str->chars = NULL;
}

void UTF8String_Resize(UTF8String *str, size_t new_capacity) {
    if (new_capacity <= str->capacity) {
        return;
    }
    UTF8Char *new_chars = realloc(str->chars, sizeof(UTF8Char) * new_capacity);
    if (new_chars == NULL) {
        logFatal("Cannot allocate memory for UTF8String.");
    }
    str->chars = new_chars;
    str->capacity = new_capacity;
}

void UTF8String_IncreaseCapacity(UTF8String *string) {
    if (string->capacity == 0) {
         UTF8String_Resize(string, UTF8STRING_INITIAL_CAPACITY);
         return;
    }
    UTF8String_Resize(string, string->capacity * UTF8STRING_GROW_FACTOR);
}

UTF8String *UTF8String_Create() {
    UTF8String *str = malloc(sizeof(UTF8String));
    UTF8String_Init(str);
    return str;
}

void UTF8String_Destroy(UTF8String *str) {
    if (str == NULL) {
        return;
    }
    UTF8String_Deinit(str);
    free(str);
}

char *UTF8String_ToStr(const UTF8String *string) {
    size_t size = 0;
    for (size_t i=0; i<string->length; i++) {
        size += string->chars[i].length;
    }
    char *return_str = malloc(sizeof(char) * size + 1);
    if (!return_str) {
        logFatal("Cannot allocate memory for return string.");
    }
    size_t idx = 0;
    for (size_t i=0; i<string->length; i++) {
        for (int j=0; j<string->chars[i].length; j++) {
            return_str[idx] = string->chars[i].bytes[j];
            idx++;
        }
    }
    return_str[idx] = '\0';
    return return_str;
}

void UTF8String_AddChar(UTF8String *str, UTF8Char ch) {
    if (str->length >= str->capacity) {
        UTF8String_IncreaseCapacity(str);
    }
    str->chars[str->length] = ch;
    str->length++;
}

void UTF8String_FromStr(UTF8String *str, const char *chstr, size_t length) {
    // Reset the string length before filling it.
    str->length = 0;
    size_t bytes_processed = 0;
    while (bytes_processed < length && *chstr != '\0') {
        UTF8Char new_char = UTF8_GetCharFromString(chstr);
        UTF8String_AddChar(str, new_char);
        chstr += new_char.length;
        bytes_processed += new_char.length;
    }
}

void UTF8String_Copy(UTF8String *dest, const UTF8String *src) {
    UTF8String_Resize(dest, src->length); // Ensures dest has enough capacity
    memmove(dest->chars, src->chars, sizeof(UTF8Char) * src->length);
    dest->length = src->length;
}

void UTF8String_Format(UTF8String *str, size_t max_length, const char *format, ...) {
    char *tmp = malloc(sizeof(char) * max_length);
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(tmp, max_length, format, args);
    va_end(args);
    UTF8String_FromStr(str, tmp, ret);
    free(tmp);
}

void UTF8String_Split(const UTF8String *s, UTF8String *a, UTF8String *b, size_t pos) {
    if (s == a || s == b || a == b) {
        logWarn("Cannot split string into itself.");
        return;
    }
    if (pos >= s->length) {
        pos = s->length;
    }
    size_t len_a = pos;
    size_t len_b = s->length - pos;
    UTF8String_Resize(a, len_a); // Ensures 'a' has enough capacity
    UTF8String_Resize(b, len_b); // Ensures 'b' has enough capacity
    memmove(a->chars, s->chars, sizeof(UTF8Char) * len_a);
    memmove(b->chars, s->chars + pos, sizeof(UTF8Char) * len_b);
    a->length = len_a;
    b->length = len_b;
}

size_t UTF8String_Length(const UTF8String *string) {
    return string->length;
}

int UTF8String_SubstringWidth(const UTF8String *string, size_t start, size_t end) {
    int w = 0;
    for (size_t i=start; i<end; i++) {
        if (i >= string->length) {
            break;
        }
        w += UTF8_GetWidth(string->chars[i]);
    }
    return w;
}

int UTF8String_Width(const UTF8String *string) {
    return UTF8String_SubstringWidth(string, 0, string->length);
}

bool UTF8String_Equal(const UTF8String *a, const UTF8String *b) {
    if (!a || !b || a->length != b->length) {
        return false;
    }
    for (size_t i=0; i<a->length; i++) {
        if (!UTF8_Equal(a->chars[i], b->chars[i])) {
            return false;
        }
    }
    return true;
}

bool UTF8String_EqualStr(const UTF8String *a, const char *b) {
    if (!a || !b) {
        return false;
    }
    char *a_str = UTF8String_ToStr(a);
    int cmp = strcmp(a_str, b);
    free(a_str);
    return cmp == 0;
}

void UTF8String_Concat(UTF8String *str1, const UTF8String *str2) {
    size_t new_length = str1->length + str2->length;
    if (new_length > str1->capacity) {
        UTF8String_Resize(str1, new_length);
    }
    // use memove to handle overlapping memory areas
    memmove(str1->chars + str1->length, str2->chars, sizeof(UTF8Char) * str2->length);
    str1->length = new_length;
}

void UTF8String_Repeat(UTF8String *str, size_t n) {
    size_t new_length = str->length * n;
    if (n == 0) {
        str->length = 0;
        return;
    }
    if (n == 1) {
        return; // Nothing to do
    }
    size_t original_length = str->length;
    if (new_length > str->capacity) {
        UTF8String_Resize(str, new_length);
    }
    for (size_t i = 1; i < n; i++) {
        memcpy(str->chars + (i * original_length), str->chars, original_length * sizeof(UTF8Char));
    }
    str->length = new_length;
}

void UTF8String_Spaces(UTF8String *str, size_t n) {
    UTF8String_FromStr(str, " ", 1);
    UTF8String_Repeat(str, n);
}

void UTF8String_Shorten(UTF8String *str, size_t n) {
    if (n >= str->length) {
        return;
    }
    str->length = n;
}

void UTF8String_SubString(const UTF8String *s, UTF8String *dest, size_t start, size_t length) {
    if (!s || !dest || start >= s->length) {
        dest->length = 0;
        return;
    }
    if (start + length > s->length) {
        length = s->length - start;
    }
    UTF8String_Resize(dest, length);
    memmove(dest->chars, s->chars + start, sizeof(UTF8Char) * length);
    dest->length = length;
}
