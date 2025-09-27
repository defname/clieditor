#include "utf8.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

const UTF8Char utf8_space = { .bytes = {' '}, .length = 1 };

// read like read() but retry if the process is interrupted.
static int robust_read(int fd, void *buf, size_t count) {
    int bytes_read = 0;

    do {
        bytes_read = read(fd, buf, count);
        if (bytes_read != -1) {
            return bytes_read;
        }
    } while (errno == EINTR);
    return -1;
}

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

UTF8Char UTF8_GetChar(int fd) {
    UTF8Char utf8_char;
    // Initialize the struct to a known, zeroed state.
    // This also sets length to 0, which is our error indicator.
    memset(&utf8_char, 0, sizeof(UTF8Char));

    unsigned char c;
    // Check for read error OR End-of-File. robust_read should return 1.
    if (robust_read(fd, &c, 1) != 1) {
        utf8_char.length = 0;
        return utf8_char;
    }
    utf8_char.bytes[0] = c;
    utf8_char.length = get_char_length(c);

    for (int i=1; i<utf8_char.length; i++) {
        // Read continuation byte
        if (robust_read(fd, &c, 1) != 1) {
            // Incomplete sequence (EOF or error)
            utf8_char.length = 0;
            return utf8_char;
        }
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

size_t UTF8_FromString(UTF8Char *buf, size_t n, const char *s) {
    size_t out_idx = 0;
    while (out_idx < n && *s != '\0') {
        buf[out_idx] = UTF8_GetCharFromString(s);
        s += buf[out_idx].length;
        out_idx++;
    }
    return out_idx;
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