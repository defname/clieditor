#include "utf8_helper.h"

#include <wchar.h>


// return the length of the utf8 char starting with c or 0 if it's invalid
size_t utf8_get_char_length(unsigned char c) {
    if (c <= 0x7F) {  // ASCII character
        return 1;
    }
    else if ((c & 0xE0) == 0xC0) {
        return 2;
    }
    else if ((c & 0xF0) == 0xE0) {
        return 3;
    }
    else if ((c & 0xF8) == 0xF0) {
        return 4;
    }
    else {
        // Invalid UTF-8 start byte.
        return 0;
    }
}

bool utf8_is_continuation_byte(unsigned char c) {
    return (c & 0xC0) == 0x80;
}

size_t utf8_strlen(const char *str) {
    size_t count = 0;
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

uint32_t utf8_to_codepoint(const char *ch) {
    if (ch == 0) return INVALID_CODEPOINT;

    size_t ch_len = utf8_get_char_length(ch[0]);
    if (ch_len == 0) return INVALID_CODEPOINT;

    for (size_t i=0; i<ch_len; i++) {
        if (!utf8_is_continuation_byte(ch[i])) {
            return INVALID_CODEPOINT;
        }
    }


    uint32_t cp = 0;

    if (ch_len == 1) {
        // ASCII
        return ch[0];
    }
    else if (ch_len == 2) {
        cp  = (ch[0] & 0x1F) << 6;  // 5 bits of first byte
        cp |= (ch[1] & 0x3F);       // 6 bits of second byte
    }
    else if (ch_len == 3) {
        cp  = (ch[0] & 0x0F) << 12; // 4 bits
        cp |= (ch[1] & 0x3F) << 6;  // 6 bits
        cp |= (ch[2] & 0x3F);       // 6 bits
    }
    else if (ch_len == 4) {
        cp  = (ch[0] & 0x07) << 18; // 3 bits
        cp |= (ch[1] & 0x3F) << 12; // 6 bits
        cp |= (ch[2] & 0x3F) << 6;  // 6 bits
        cp |= (ch[3] & 0x3F);       // 6 bits
    }
    else {
        return INVALID_CODEPOINT;
    }

    return cp;
}

size_t utf8_from_codepoint(uint32_t cp, char *out) {
    if (cp <= 0x7F) {
        out[0] = cp;
        return 1;
    }
    if (cp <= 0x07FF) {
        out[0] = cp >> 7 | 0xC0;
        out[1] = (cp & 0x3F) | 0x80;
        return 2;
    }
    if (cp <= 0xFFFF) {
        out[0] = cp >> 12 | 0xE0;
        out[1] = (cp >> 6 & 0x3F) | 0x80;
        out[2] = (cp & 0x3F) | 0x80;
        return 3;
    }
    if (cp <= 0x10FFFF) {
        out[0] = cp >> 18 | 0xF0;
        out[1] = (cp >> 12 & 0x3F) | 0x80;
        out[1] = (cp >> 6 & 0x3F) | 0x80;
        out[1] = (cp & 0x3F) | 0x80;
        return 4;
    }
    return 0;

}


int utf8_get_width(uint32_t cp) {
    int w = wcwidth((wchar_t)cp);
    if (w < 0) w = 1; // fallback
    return w;
}