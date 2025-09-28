#ifndef UTF8STRING_H
#define UTF8STRING_H

#include "utf8.h"

#define UTF8STRING_GROW_FACTOR 2
#define UTF8STRING_INITIAL_CAPACITY 16


typedef struct _UTF8String {
    UTF8Char *chars;
    size_t length;
    size_t capacity;
} UTF8String;

void UTF8String_Init(UTF8String *string);
void UTF8String_Deinit(UTF8String *string);
void UTF8String_Resize(UTF8String *string, size_t new_capacity);
void UTF8String_IncreaseCapacity(UTF8String *string);

UTF8String *UTF8String_Create();
void UTF8String_Free();

void UTF8String_AddChar(UTF8String *str, UTF8Char ch);
void UTF8String_FromStr(UTF8String *str, const char *chstr, size_t length);

#endif