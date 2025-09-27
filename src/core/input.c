#include "input.h"

#include <string.h>
#include "utf8.h"
#include "../utils/logging.h"


UTF8Char buffer[INPUT_BUFFER_SIZE] = {0};
size_t buffer_pos = 0;

const char *escape_sequences[] = {
    "\e[A",     // ESC_CURSOR_UP
    "\e[B",     // ESC_CURSOR_DOWN
    "\e[C",     // ESC_CURSOR_RIGHT
    "\e[D"      // ESC_CURSOR_LEFT
};

// to store matching sequences
int results[ESC_NONE] = {0};
int results_count = 0;
EscapeSequence result;


void filter_possible_sequences(int seq_idx) {
    int new_idx_count = 0;
    for (int i=0; i<results_count; i++) {
        if (escape_sequences[results[i]][seq_idx] == buffer[seq_idx].bytes[0]) {
            results[new_idx_count] = results[i];
            new_idx_count++;
        }
    }
    results_count = new_idx_count;
}

void reset_results() {
    for (int i=0; i<ESC_NONE; i++) {
        results[i] = i;
    }
    results_count = ESC_NONE;
    result = ESC_NONE;
}

void reset_buffer() {
    buffer_pos = 0;
    memset(buffer, 0, sizeof(buffer));
}

EscapeSequence Input_Read(int fd) {
    reset_buffer();
    reset_results();

    for (int i=0; ; i++) {
        buffer[i] = UTF8_GetChar(fd);
        if (buffer[i].length != 1) {
            return ESC_NONE;
        }
        filter_possible_sequences(i);
        if (results_count == 0) {
            return ESC_NONE;
        }
        if (results_count == 1) {
            result = results[0];
            return result;
        }
    }
}

UTF8Char Input_GetChar() {
    if (buffer_pos >= INPUT_BUFFER_SIZE) {
        logFatal("Input buffer overflow.");
    }
    return buffer[buffer_pos++];
}
