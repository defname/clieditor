#ifndef UTF8STRING_H
#define UTF8STRING_H

#include "common/utf8.h"

#define UTF8STRING_GROW_FACTOR 2
#define UTF8STRING_INITIAL_CAPACITY 16


typedef struct _UTF8String {
    UTF8Char *chars;
    size_t length;
    size_t capacity;
} UTF8String;

// Initialization and capacity management (only init/deinit should be needed)
void UTF8String_Init(UTF8String *string);
void UTF8String_Deinit(UTF8String *string);
void UTF8String_Resize(UTF8String *string, size_t new_capacity);
void UTF8String_IncreaseCapacity(UTF8String *string);

// Creation
UTF8String *UTF8String_Create();
void UTF8String_Destroy(UTF8String *string);

// Properties
size_t UTF8String_Length(const UTF8String *string);
int UTF8String_SubstringWidth(const UTF8String *string, size_t start, size_t end);
int UTF8String_Width(const UTF8String *string);

// Conversion
char *UTF8String_ToStr(const UTF8String *string);  // return value needs to be freed by the caller

// Filling with content
void UTF8String_AddChar(UTF8String *str, UTF8Char ch);
void UTF8String_FromStr(UTF8String *str, const char *chstr, size_t length);
void UTF8String_Copy(UTF8String *dest, const UTF8String *src);

// Manipulation
void UTF8String_Concat(UTF8String *str1, const UTF8String *str2);
void UTF8String_Split(const UTF8String *s, UTF8String *a, UTF8String *b, size_t pos);
void UTF8String_Repeat(UTF8String *str, size_t n);
void UTF8String_Spaces(UTF8String *str, size_t n);
void UTF8String_Shorten(UTF8String *str, size_t n);

#endif