/**
 * input.h
 * Read and buffer input to recognize ESC sequences.
 * 
 * Usage:
 * 
 * EscapeSequence e = Input_Read()  // read from terminal
 * 
 * while ((UTF8Char ch = Input_GetChar()).length > 0) {
 *     handleInputChar(ch);
 * }
 */
#ifndef INPUT_H
#define INPUT_H

#include "utf8.h"

#define INPUT_BUFFER_SIZE   8

typedef enum {
    ESC_CURSOR_UP,
    ESC_CURSOR_DOWN,
    ESC_CURSOR_RIGHT,
    ESC_CURSOR_LEFT,
    ESC_NONE
} EscapeSequence;

EscapeSequence Input_Read(int fd);  // use UTF8_GetChar() to read input. Return the found escape sequence or ESC_NONE
UTF8Char Input_GetChar();           // get the next character from buffer (0-length character is returned if buffer is empty)


#endif