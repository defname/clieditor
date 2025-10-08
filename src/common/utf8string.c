#include "common/utf8string.h"

#include <stdlib.h>
#include <string.h>

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

int UTF8String_SubStringWidth(const UTF8String *string, size_t start, size_t end) {
    int w = 0;
    for (size_t i=start; i<end; i++) {
        if (i >= string->length) {
            break;
        }
        w += UTF8_GetWidth(string->chars[i]);
    }
    return w;
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