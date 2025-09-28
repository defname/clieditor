#include "utf8string.h"

#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"


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
    UTF8String_Resize(string, string->capacity * UTF8STRING_GROW_FACTOR);
}

UTF8String *UTF8String_Create() {
    UTF8String *str = malloc(sizeof(UTF8String));
    UTF8String_Init(str);
    return str;
}

void UTF8String_Free(UTF8String *str) {
    if (str == NULL) {
        return;
    }
    UTF8String_Deinit(str);
    free(str);
}

void UTF8String_AddChar(UTF8String *str, UTF8Char ch) {
    if (str->length >= str->capacity) {
        UTF8String_IncreaseCapacity(str);
    }
    str->chars[str->length] = ch;
    str->length++;
}

void UTF8String_FromStr(UTF8String *str, const char *chstr, size_t length) {
    UTF8String_Resize(str, length);
    size_t out_idx = 0;
    while (out_idx < length && *chstr != '\0') {
        UTF8Char new_char = UTF8_GetCharFromString(chstr);
        chstr += new_char.length;
        out_idx++;
        UTF8String_AddChar(str, new_char);
    }
}

