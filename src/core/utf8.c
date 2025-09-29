#include "utf8.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

const UTF8Char utf8_space = { .bytes = {' '}, .length = 1 };
const UTF8Char utf8_invalid = { .bytes = {0}, .length = 0 };


// return the length of the utf8 char starting with c or 0 if it's invalid
size_t get_char_length(unsigned char c) {
    if (c <= 127) {  // ASCI character
        return 1;
    }
    else if ((c & 0b11100000) == 0b11000000) {
        return 2;
    }
    else if ((c & 0b11110000) == 0b11100000) {
        return 3;
    }
    else if ((c & 0b11111000) == 0b11110000) {
        return 4;
    }
    else {
        // Invalid UTF-8 start byte.
        return 0;
    }
}

UTF8Char UTF8_ReadCharFromBuf(Buffer *buf) {
    UTF8Char out = (UTF8Char){ .bytes = {0}, .length = 0 };

    if (Buffer_IsEmpty(buf)) {
        return out;
    }
    unsigned char c;
    Buffer_Peek(buf, 0, &c);
    size_t len = get_char_length(c);

    if (len > Buffer_Size(buf)) {
        return out;
    }

    Buffer_Dequeue(buf, NULL);

    out.bytes[0] = c;
    out.length = len;

    size_t i = 1;
    for (; i<len; i++) {
        // Read continuation byte
        Buffer_Dequeue(buf, &c);
        // Validate that it's a valid continuation byte (starts with 10xxxxxx)
        if ((c & 0b11000000) != 0b10000000) {
            // Invalid sequence
            out.length = 0;
            
            return out;
        }
        out.bytes[i] = c;
    }
    return out;
}

UTF8Char UTF8_GetCharFromString(const char *s) {
    UTF8Char utf8_char;
    // Initialize the struct to a known, zeroed state.
    // This also sets length to 0, which is our error indicator.
    memset(&utf8_char, 0, sizeof(UTF8Char));

    unsigned char c = s[0];

    utf8_char.bytes[0] = c;
    utf8_char.length = get_char_length(c);

    for (int i=1; i<utf8_char.length; i++) {
        // Read continuation byte
        c = s[i];
        // Validate that it's a valid continuation byte (starts with 10xxxxxx)
        if ((c & 0b11000000) != 0b10000000) {
            // Invalid sequence
            utf8_char.length = 0;
            return utf8_char;
        }
        utf8_char.bytes[i] = c;
    }
    return utf8_char;

}

ssize_t UTF8_PutChar(int fd, UTF8Char ch) {
    if (ch.length == 0) {
        return 0;
    }
    return write(fd, ch.bytes, ch.length);
}

bool UTF8_Equal(UTF8Char a, UTF8Char b) {
    if (a.length != b.length) {
        return false;
    }
    return memcmp(a.bytes, b.bytes, a.length) == 0;
}

bool UTF8_EqualToChar(UTF8Char a, char b) {
    if (a.length != 1) {
        return false;
    }
    return a.bytes[0] == b;
}

bool UTF8_IsPrintable(UTF8Char ch) {
    if (ch.length == 0) {
        return false;
    }
    if (ch.length == 1) {
        return isprint(ch.bytes[0]);
    }
    // TODO!!!!
    return true;
}