/**
 * utf8.h
 * Function to read and write UTF8 characters and strings.
 */
#ifndef UTF8_H
#define UTF8_H

#include <stdbool.h>
#include <unistd.h>

#include "utils/buffer.h"


typedef struct _UTF8Char {
    unsigned char bytes[4];
    char length;
} UTF8Char;

extern const UTF8Char utf8_space;
extern const UTF8Char utf8_invalid;

UTF8Char UTF8_GetCharFromString(const char *s); // Read an UTF8 character from a string
size_t UTF8_FromString(UTF8Char *buf, size_t n, const char *s); // Read an UTF8 string

UTF8Char UTF8_ReadCharFromBuf(Buffer *buf); 

ssize_t UTF8_PutChar(int fd, UTF8Char ch);      // Write an UTF character to fd (return the number of bytes written or -1 on failure)
bool UTF8_Equal(UTF8Char a, UTF8Char b);        // return true if a and b are equal
bool UTF8_EqualToChar(UTF8Char a, char b);      // return true if a and b are equal


#endif