#ifndef UTF8_HELPER_H
#define UTF8_HELPER_H

#define INVALID_CODEPOINT 0xFFFFFFFF

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


size_t utf8_get_char_length(unsigned char c);
bool utf8_is_continuation_byte(unsigned char c);
size_t utf8_strlen(const char *str);
uint32_t utf8_get_codepoint(const char *ch);
int utf8_get_width(const char *ch);

#endif