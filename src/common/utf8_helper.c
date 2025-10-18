#include "utf8_helper.h"

#include <ctype.h>
#include <wchar.h>


// return the length of the utf8 char starting with c or 0 if it's invalid
size_t utf8_get_char_length(unsigned char c) {
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

bool utf8_is_continuation_byte(unsigned char c) {
    return (c & 0b11000000) == 0b10000000;
}

size_t utf8_strlen(const char *str) {
    int count = 0;
    const char *byte = str;
    if (!byte) {
        return 0;
    }
    while (*byte != '\0') {
        size_t ch_len = utf8_get_char_length(*byte);
        if (ch_len == 0) break; // Stop on invalid char or end of string
        byte += ch_len;
        count++;
    }
    return count;
}

uint32_t utf8_get_codepoint(const char *ch) {
    if (ch == 0) return INVALID_CODEPOINT;

    size_t ch_len = utf8_get_char_length(ch[0]);
    if (ch_len == 0) return INVALID_CODEPOINT;

    uint32_t cp = 0;

    if (ch_len == 1) {
        // ASCII
        return ch[0];
    }
    else if (ch_len == 2) {
        cp  = (ch[0] & 0b00011111) << 6;  // 5 bits aus erstem Byte
        cp |= (ch[1] & 0b00111111);       // 6 bits aus zweitem Byte
    }
    else if (ch_len == 3) {
        cp  = (ch[0] & 0b00001111) << 12; // 4 bits
        cp |= (ch[1] & 0b00111111) << 6;  // 6 bits
        cp |= (ch[2] & 0b00111111);       // 6 bits
    }
    else if (ch_len == 4) {
        cp  = (ch[0] & 0b00000111) << 18; // 3 bits
        cp |= (ch[1] & 0b00111111) << 12; // 6 bits
        cp |= (ch[2] & 0b00111111) << 6;  // 6 bits
        cp |= (ch[3] & 0b00111111);       // 6 bits
    }
    else {
        return INVALID_CODEPOINT;
    }

    return cp;
}

int utf8_get_width(const char *ch) {
    uint32_t cp = utf8_get_codepoint(ch);
    int w = wcwidth(cp);
    if (w < 0) w = 1; // fallback
    return w;
}