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
#include "common/string.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "common/logging.h"
#include "utf8_helper.h"

/*****************************************************************************/
/* StringView                                                                */

StringView StringView_FromLiteral(const char *str) {
    return (StringView){
        .bytes = str,
        .bytes_size = strlen(str),
        .char_count = utf8_strlen(str)
    };
}

size_t StringView_Length(const StringView *view) {
    return view->char_count;
}

size_t StringView_Width(const StringView *view) {
    StringIterator it = StringIterator_FromView(view);
    int w = 0;
    while (StringIterator_Next(&it)) {
        w += utf8_calc_width(utf8_to_codepoint(it.current));
    }
    return w;
}

bool StringView_Equal(const StringView *a, const StringView *b) {
    if (a->bytes_size != b->bytes_size) {
        return false;
    }
    return StringView_EqualToStr(a, b->bytes, b->bytes_size);
}

bool StringView_EqualToStr(const StringView *view, const char *cstr, size_t length) {
    if (view->bytes_size != length) {
        return false;
    }
    return memcmp(view->bytes, cstr, length) == 0;
}


/*****************************************************************************/
/* StringIterator                                                            */

StringIterator StringIterator_FromString(const String *str) {
    StringView view = String_ToView(str);
    return StringIterator_FromView(&view);
}

StringIterator StringIterator_FromView(const StringView *str) {
    return (StringIterator){
        .bytes = str->bytes,
        .bytes_size = str->bytes_size,

        .byte_offset = 0,
        .char_index = 0,
        .current = NULL
    };
}

bool StringIterator_Next(StringIterator *it) {
    if (!it || !it->bytes) {
        logWarn("Invalid StringIterator.");
        return false;
    }
    // first call
    if (!it->current) {
        if (it->bytes_size == 0) {
            return false;
        }
        it->current = it->bytes;
        it->byte_offset = 0;
        it->char_index = 0;
        return true;
    }

    // following calls
    size_t ch_len = utf8_get_char_length(it->current[0]);
    if (ch_len == 0) return false;  // ungültiges UTF-8

    it->byte_offset += ch_len;
    it->char_index++;

    if (it->byte_offset >= it->bytes_size) return false;

    it->current = it->bytes + it->byte_offset;
    return true;
}


/*****************************************************************************/
/* Helper functions                                                          */

/**
 * Increase ptr capacity
 */
void *increase_ptr_capacity(void *ptr, size_t new_capacity, size_t old_capacity, size_t element_size) {
    if (new_capacity <= old_capacity) {
        return ptr;
    }
    void *new_ptr = realloc(ptr, element_size * new_capacity);
    if (new_ptr == NULL) {
        logFatal("Cannot allocate memory for String.");
    }
    return new_ptr;
}

/**
 * Resize capacity of str to new_capacity 
 */
void resize_bytes_capacity(String *str, size_t new_capacity) {
    if (new_capacity <= str->bytes_capacity) {
        return;
    }   
    str->bytes = increase_ptr_capacity(str->bytes, new_capacity, str->bytes_capacity, sizeof(char));
    str->bytes_capacity = new_capacity;
}

/**
 * Increase capactiy of string 
 */
void increase_bytes_capacity(String *string) {
    if (string->bytes_capacity == 0) {
         resize_bytes_capacity(string, STRING_INITIAL_CAPACITY);
         return;
    }
    resize_bytes_capacity(string, STRING_GROW(string->bytes_capacity));
}

void resize_multibytes_capacity(String *str, size_t new_capacity) {
    if (new_capacity <= str->multibytes_capacity) {
        return;
    }   
    str->multibytes = increase_ptr_capacity(str->multibytes, new_capacity, str->multibytes_capacity, sizeof(MultibyteIndexHelper));
    str->multibytes_capacity = new_capacity;
}

/**
 * Increase capacity of multibyte_offsets array
 */
void increase_multibytes_capacity(String *string) {
    if (string->multibytes == NULL || string->multibytes_capacity == 0) {
        resize_multibytes_capacity(string, STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY);
        return;
    }
    resize_multibytes_capacity(string, STRING_GROW(string->multibytes_capacity));
}

/**
 * Add an offset to string->multibyte_offsets and increase capacity if needed.
 */
void add_multibyte_offset(String *string, size_t offset, size_t length) {
    if (string->multibytes_size + 1 > string->multibytes_capacity) {
        increase_multibytes_capacity(string);
    }
    MultibyteIndexHelper *mih = &string->multibytes[string->multibytes_size];

    if (string->multibytes_size == 0) {
        mih->char_index = offset + 1;
        mih->byte_offset = offset + length;
    }
    else {
        MultibyteIndexHelper *prev = &string->multibytes[string->multibytes_size - 1];
        // char index after last multibyte char + byte distance between + 1 (to end after current multibyte char)
        mih->char_index = prev->char_index + (offset - prev->byte_offset) + 1;
        mih->byte_offset = offset + length;
    }
    string->multibytes_size++;
}

/**
 * Rebuild the complete multibyte_offsets array
 */
void rebuild_multibyte_offsets(String *string) {
    if (!string->multibytes_invalid) {
        return;
    }
    string->multibytes_size = 0;
    StringIterator it = StringIterator_FromString(string);
    while (StringIterator_Next(&it)) {
        size_t ch_len = utf8_get_char_length(*it.current);
        if (ch_len == 0) {
            break;
        }
        if (ch_len == 1) {
            continue;
        }
        add_multibyte_offset(string, it.byte_offset, ch_len);
    }
    string->multibytes_invalid = false;
}


/*****************************************************************************/
/* String                                                                */

void String_Init(String *str) {
    str->bytes = NULL;
    str->bytes_capacity = 0;
    str->bytes_size = 0;

    str->multibytes = NULL;
    str->multibytes_capacity = 0;
    str->multibytes_size = 0;
    str->multibytes_invalid = false;

    str->char_count = 0;

    resize_bytes_capacity(str, STRING_INITIAL_CAPACITY);
    str->bytes[0] = '\0';
}

void String_Deinit(String *str) {
    if (str->bytes) {
        free(str->bytes);
    }
    if (str->multibytes) {
        free(str->multibytes);
    }
    str->bytes = NULL;
    str->bytes_capacity = 0;
    str->bytes_size = 0;

    str->multibytes = NULL;
    str->multibytes_capacity = 0;
    str->multibytes_size = 0;
    str->multibytes_invalid = false;

    str->char_count = 0;
}


String *String_Create() {
    String *str = malloc(sizeof(String));
    if (!str) {
        logFatal("Cannot allocate memory for String.");
    }
    String_Init(str);
    return str;
}

void String_Destroy(String *str) {
    if (str == NULL) {
        return;
    }
    String_Deinit(str);
    free(str);
}

size_t String_Length(const String *string) {
    return string->char_count;
}

const char *String_AsCStr(const String *string) {
    return string->bytes;
}

String String_FromCStr(const char *chstr, size_t length) {
    String str;
    String_Init(&str);
    if (!chstr) {
        return str;
    }
    if (str.bytes_capacity < length + 1) {
        resize_bytes_capacity(&str, length + 1);
    }
    str.bytes_size = length;
    memcpy(str.bytes, chstr, length);  // do not copy '\0' (might be chopped by length)
    str.bytes[length] = '\0';

    str.char_count = utf8_strlen(str.bytes);
    str.multibytes_invalid = true;
    return str;
}

void String_AddChar(String *str, const char *ch) {
    if (!ch) {
        return;
    }
    size_t ch_len = utf8_get_char_length(ch[0]);
    if (ch_len == 0) {
        return;
    }
    if (str->bytes_size + ch_len + 1 > str->bytes_capacity) {
        increase_bytes_capacity(str);
    }
    str->bytes[str->bytes_size] = ch[0];

    for (size_t i=1; i<ch_len; i++) {
        if (!utf8_is_continuation_byte(ch[i])) {
            str->bytes[str->bytes_size] = '\0';
            return;
        }
        str->bytes[str->bytes_size + i] = ch[i];
    }
    // update multibyte_offsets if needed
    // do not update the multibytes_invalid flag. if it was invalid before
    // it is still if it was valid it's also still
    if (ch_len > 1) {
        add_multibyte_offset(str, str->bytes_size, ch_len);
    }
    str->bytes_size += ch_len;
    str->bytes[str->bytes_size] = '\0';
    str->char_count++;
}

String String_Copy(const String *src) {
    return String_FromCStr(src->bytes, src->bytes_size);
}

String String_Format(const char *format, ...) {
    // calculate the length of the result
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        logError("Encoding error in String_Format.");
        return String_Repeat(NULL, 0);  // empty string
    }

    String str;
    String_Init(&str);
    resize_bytes_capacity(&str, needed + 1);
    va_start(args, format);
    vsnprintf(str.bytes, needed + 1, format, args);
    va_end(args);

    str.bytes_size = needed;
    str.char_count = utf8_strlen(str.bytes);
    str.multibytes_invalid = true;

    return str;
}

String String_FromView(StringView view) {
    return String_FromCStr(view.bytes, view.bytes_size);
}

StringView String_ToView(const String *string) {
    return (StringView){
        .bytes = string->bytes,
        .bytes_size = string->bytes_size,
        .char_count = string->char_count
    };
}

const char *String_GetChar(String *str, int pos) {
    // 1. Handle edge case: empty string
    if (str->char_count == 0) {
        return NULL;
    }

    if (str->multibytes_invalid) {
        rebuild_multibyte_offsets(str);
    }

    // 2. Normalize position for modulo and negative indices
    pos = pos % (int)str->char_count;  // make sure to stay inside the string
    //          ^^^^^ casting size_t to int is important to prevent loosing the sign
    if (pos < 0) {  // support negative indexing to count from the end
        pos += str->char_count;
    }

    // 3. Handle simple case: no multibyte characters
    if (str->multibytes_size == 0) {
        return str->bytes + pos;
    }

    // 4. Perform binary search to find the correct offset entry.
    // This is a variant of `upper_bound`. It finds the first entry
    // whose `char_index` is strictly greater than `pos`.
    size_t i1 = 0;
    size_t i2 = str->multibytes_size;
    while (i1 < i2) {
        size_t mid = (i1 + i2) / 2;
        MultibyteIndexHelper *offset = &str->multibytes[mid];
        if (offset->char_index <= (size_t)pos) {
            i1 = mid + 1;
        }
        else {
            i2 = mid;
        }
    }

    // 5. Calculate the final byte position.
    // If i1 is 0, it means `pos` is before the end of the first multibyte block.
    // In this case, the byte offset is simply the character position.
    if (i1 == 0) {
        return str->bytes + pos;
    }

    // Otherwise, `pos` is after at least one multibyte block.
    // The relevant index entry is the one *before* `i1`.
    MultibyteIndexHelper *offset = &str->multibytes[i1 - 1];
    size_t byte_pos = offset->byte_offset + ((size_t)pos - offset->char_index);
    return str->bytes + byte_pos;
}

StringView String_Slice(String *string, size_t start, size_t end) {
    StringView view;
    if (start >= string->char_count || end > string->char_count || start > end) {
        view.bytes = NULL;
        view.bytes_size = 0;
        view.char_count = 0;
        return view;
    }
    view.bytes = String_GetChar(string, start);
    const char *end_char = end == string->char_count ? string->bytes + string->bytes_size : String_GetChar(string, end);
    view.bytes_size = end_char - view.bytes;
    view.char_count = end - start;
    return view;
}

String String_Substring(String *s, size_t start, size_t length) {
    if (start >= s->char_count || length == 0) {
        return String_FromCStr("", 0);
    }
    if (start + length > s->char_count) {
        length = s->char_count - start;
    }
    return String_FromView(String_Slice(s, start, start + length));
}

void String_Append(String *str1, const String *str2) {
    size_t new_byte_size = str1->bytes_size + str2->bytes_size;
    if (new_byte_size > str1->bytes_capacity) {
        resize_bytes_capacity(str1, new_byte_size + 1);
    }
    // use memove to handle overlapping memory areas
    memmove(str1->bytes + str1->bytes_size, str2->bytes, sizeof(char) * str2->bytes_size);
    str1->bytes_size = new_byte_size;
    str1->bytes[new_byte_size] = '\0';
    str1->char_count += str2->char_count;
    str1->multibytes_invalid = true;
}

String String_Repeat(const String *str, size_t n) {
    String out;
    String_Init(&out);
    if (n == 0 || !str) {
        return out;
    }
    if (n == 1) {
        return String_Copy(str);
    }
    size_t new_bytes_size = str->bytes_size * n;
    
    size_t old_bytes_size = str->bytes_size;
    resize_bytes_capacity(&out, new_bytes_size + 1);

    for (size_t i = 0; i < n; i++) {
        memcpy(out.bytes + (i * old_bytes_size), str->bytes, old_bytes_size * sizeof(char));
    }
    out.bytes_size = new_bytes_size;
    out.bytes[new_bytes_size] = '\0';
    out.char_count = n * str->char_count;
    out.multibytes_invalid = true;

    return out;
}

String String_Spaces(size_t n) {
    return String_Format("%*s", n, "");
}

void String_Shorten(String *str, size_t n) {
    if (n >= str->bytes_size) {
        return;
    }
    str->bytes_size = n;
    str->bytes[n] = '\0';
    str->char_count = utf8_strlen(str->bytes);
    // even if the entries in multibytes are still valid multibytes_size
    // changed and lookups would do unnecessary iterations
    str->multibytes_invalid = true;
}
