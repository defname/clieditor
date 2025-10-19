/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "input.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "common/utf8_helper.h"
#include "io/terminal.h"
#include "common/logging.h"
#include "common/buffer.h"
/*
CSI ::= ESC '['

csi_sequence ::= CSI [ parameters ] [ intermediates ] final_byte
parameters   ::= (parameter_byte)*
parameter_byte ::= 0x30–0x3F  // '0'..'?'  → Ziffern, Semikolon, ...
intermediates ::= (intermediate_byte)*
intermediate_byte ::= 0x20–0x2F  // ' '..'/' → meist selten
final_byte   ::= 0x40–0x7E       // '@'..'~' → bestimmt die Funktion
*/

/*
CSI sequences for keys

ESC [ key ~
for single keys

ESC [ key ; modifier ~
for key combinations

Modifier        Value
Shift	        2
Alt	            3
Shift+Alt	    4
Ctrl	        5
Shift+Ctrl	    6
Alt+Ctrl	    7
Shift+Alt+Ctrl	8
*/


InputEvent input_invalidevent = { .key = KEY_NONE, .mods = 0, .ch = INVALID_CODEPOINT };


#define MAX_SEQUENCE_LEN    6


bool InputEvent_IsValid(const InputEvent *ev) {
    if (ev->key == KEY_CHAR) {
        return ev->ch != 0x00 && ev->ch != INVALID_CODEPOINT;
    }
    if (ev->key != KEY_NONE) {
        return true;
    }
    // accept e.g. for chars with mods
    return utf8_is_ascii(ev->ch);
}

void Input_Init() {
}

void Input_Deinit() {
}

// helper: read mit Timeout (ms)
static ssize_t read_with_timeout(int fd, unsigned char *c, int timeout_ms) {
    fd_set set;
    struct timeval tv;

    FD_ZERO(&set);
    FD_SET(fd, &set);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int rv = select(fd + 1, &set, NULL, NULL, &tv);
    if (rv == -1) return -1;  // error look in errno
    if (rv == 0) return 0; // Timeout

    return read(fd, c, 1);
}

static bool choose_key(char code, InputEvent *ev) {
    switch (code) {
        case 'A':
            ev->key = KEY_UP;
            break;
        case 'B':
            ev->key = KEY_DOWN;
            break;
        case 'C':
            ev->key = KEY_RIGHT;
            break;
        case 'D':
            ev->key = KEY_LEFT;
            break;
        case 'H':
            ev->key = KEY_HOME;
            break;
        case 'F':
            ev->key = KEY_END;
            break;
        case '5':
            ev->key = KEY_PAGE_UP;
            break;
        case '6':
            ev->key = KEY_PAGE_DOWN;
            break;
        case '2':
            ev->key = KEY_INSERT;
            break;
        case '3':
            ev->key = KEY_DELETE;
            break;
        default:
            return false;
    }
    return true;
}

static bool choose_mod(char code, InputEvent *ev) {
    ev->mods = 0;
    switch (code) {
        case '1':
            return true;  // explicit "no modifiers"
        case '2':
            ev->mods |= KEY_MOD_SHIFT;
            break;
        case '3':
            ev->mods |= KEY_MOD_ALT;
            break;
        case '4':
            ev->mods |= KEY_MOD_ALT | KEY_MOD_SHIFT;
            break;
        case '5':
            ev->mods |= KEY_MOD_CTRL;
            break;
        case '6':
            ev->mods |= KEY_MOD_CTRL | KEY_MOD_SHIFT;
            break;
        case '7':
            ev->mods |= KEY_MOD_ALT | KEY_MOD_CTRL;
            break;
        case '8':
            ev->mods |= KEY_MOD_SHIFT | KEY_MOD_ALT | KEY_MOD_CTRL;
            break;
        default:
            return false;
    }
    return true;
}

static InputEvent build_event_from_seq(const unsigned char *seq, size_t param_len, size_t intermediate_len) {
    // if this function is used seq starts with "\e[", so seq[2] is the first param or final byte
    InputEvent ev = input_invalidevent;

    size_t final_idx = 2 + param_len + intermediate_len;
    if (final_idx >= MAX_SEQUENCE_LEN) {
        return input_invalidevent;
    }
    unsigned char final_byte = seq[final_idx];
    unsigned char key_token = final_byte == '~' ? seq[2] : final_byte;


    if ((param_len == 0 || param_len == 1) && intermediate_len == 0) {  // single key
        if (choose_key(key_token, &ev)) {
            return ev;
        }
        return input_invalidevent;
    }

    if (param_len >= 3 && seq[3] == ';') {
        if (choose_key(key_token, &ev) && choose_mod(seq[4], &ev)) {
            return ev;
        }
        return input_invalidevent;
    }

    return ev;
}


static bool is_param_byte(unsigned char c) {
    return 0x30 <= c && c <= 0x3F;
}

static bool is_intermediate_byte(unsigned char c) {
    return 0x20 <= c && c <= 0x2F;
}

static bool is_final_byte(unsigned char c) {
    return 0x40 <= c && c <= 0x7E;
}

// read an escape sequence. its assumed that the '\e' was already read!
InputEvent read_escape_sequence(int fd) {
    unsigned char seq[MAX_SEQUENCE_LEN] = { '\e' };
    size_t capacity = MAX_SEQUENCE_LEN;
    size_t seq_count = 1;

    ssize_t bytes_read = read_with_timeout(fd, &seq[1], 50);
    if (bytes_read != 1) {  // single press  of escape button
        return (InputEvent){ .key = KEY_ESC, .mods = 0, .ch = 0x00 };
    }
    (seq_count)++;  // bytes_read == 1
    if (seq[1] != '[') {  // single byte escape sequence
        return (InputEvent){
            .key = KEY_NONE,
            .mods = KEY_MOD_ALT,
            .ch = seq[1]  // for ASCII character the utf8 codepoint equals the ASCII code
        };
    }

    // so far we read '\e' in the calling function Input_Read() and '['
    // so
    // tmp_buf == { '\e', '[' }
    // tmp_buf_count == 2

    enum { PARAMS, INTER, FINAL } state = PARAMS;
    size_t param_len = 0;
    size_t intermediate_len = 0;

    while (seq_count < capacity) {
        unsigned char ch;
        // read byte from input
        bytes_read = read_with_timeout(fd, &ch, 100);
        if (bytes_read != 1) {
            // timeout, the sequence is incomplete
            return input_invalidevent;
        }
        if (state == PARAMS) {
            if (is_param_byte(ch)) {
                seq[seq_count++] = ch;
                param_len++;
                continue;
            }
            state++;
        }
        if (state == INTER) {
            if (is_intermediate_byte(ch)) {
                seq[seq_count++] = ch;
                intermediate_len++;
                continue;
            }
            state++;
        }
        if (state == FINAL) {
            if (is_final_byte(ch)) {
                seq[seq_count++] = ch;
                return build_event_from_seq(seq, param_len, intermediate_len);
            }
            seq[seq_count++] = ch;
            return input_invalidevent;
        }
    }

    logError("Read sequence exceeded capacity.");
    return input_invalidevent;
}


static size_t get_utf8_length(unsigned char c) {
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
    // Invalid UTF-8 start byte.
    return 0;
}


static void clear_input_buffer() {
    tcflush(terminal.fd_in, TCIFLUSH);
}

static InputEvent return_helper(InputEvent ev) {
    clear_input_buffer();
    return ev;
}

InputEvent Input_Read() {

    int fd = terminal.fd_in;

    // read the first byte
    unsigned char c;
    ssize_t byted_read = read(fd, &c, 1);
    if (byted_read != 1) {
        return return_helper(input_invalidevent);
    }

    // if it is not the potential beginning of an escape sequence
    if (c != '\e') {
        // check if it is an utf8 multi byte character and read it
        char utf8_buf[5] = { c, 0, 0, 0, 0 };
        int l = get_utf8_length(c);
        if (l == 0) {
            return return_helper(input_invalidevent);
        }
        for (int i=1; i<l; i++) {
            ssize_t bytes_read = read(fd, &utf8_buf[i], 1);  // this should be more robust as trying to read all l-1 bytes at once
            if (bytes_read != 1) {
                return return_helper(input_invalidevent);
            }
        }
        InputEvent ev = {
            .key = KEY_CHAR,
            .mods = 0,
            .ch = utf8_to_codepoint(utf8_buf)
        };
        // set key for enter, backspace and keys that are delivered inconsistantly
        if (l == 1) {
            switch (c) {
                case '\n':
                case '\r':
                    ev.key = KEY_ENTER;
                    break;
                case 127:
                case '\b':
                    ev.key = KEY_BACKSPACE;
                    break;
                default:
                    break;
            }
        }
        return return_helper(ev);
    }

    // read the escape sequence
    InputEvent ev = read_escape_sequence(fd);

    return return_helper(ev);
}
