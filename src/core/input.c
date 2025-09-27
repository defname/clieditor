#include "input.h"

#include <stdbool.h>
#include <string.h>
#include "utf8.h"
#include "../utils/logging.h"


// rotating buffer
UTF8Char buffer[INPUT_BUFFER_SIZE] = {0};
size_t buffer_start = 0;  // starting position in buffer
size_t buffer_count = 0;  // number of elements in buffer

const char *escape_sequences[] = {
    "\e[A",     // ESC_CURSOR_UP
    "\e[B",     // ESC_CURSOR_DOWN
    "\e[C",     // ESC_CURSOR_RIGHT
    "\e[D"      // ESC_CURSOR_LEFT
};

// to store matching sequences
int results[ESC_NONE] = {0};
int results_count = 0;

// filter the results by removing all entries which the seq_idx character doesn't match the last input char
void filter_possible_sequences(const UTF8Char *temp_buf, int seq_idx) {
    int new_idx_count = 0;
    for (int i = 0; i < results_count; i++) {
        if (escape_sequences[results[i]][seq_idx] == temp_buf[seq_idx].bytes[0]) {
            results[new_idx_count] = results[i];
            new_idx_count++;
        }
    }
    results_count = new_idx_count;
}

// reset the results
void reset_results() {
    for (int i = 0; i < ESC_NONE; i++) {
        results[i] = i;
    }
    results_count = ESC_NONE;
}

// check if there is an entry in results that finishes right at this position
int check_for_match(int seq_idx) {
    for (int i = 0; i < results_count; i++) {
        if (escape_sequences[results[i]][seq_idx] == '\0') {
            return results[i];
        }
    }
    return -1;
}

// Move characters from a temporary buffer to the main ring buffer
void push_to_buffer(const UTF8Char *temp_buf, int count) {
    if (buffer_count + count > INPUT_BUFFER_SIZE) {
        logFatal("Input buffer overflow on push.");
        return;
    }
    for (int i = 0; i < count; i++) {
        int buf_idx = (buffer_start + buffer_count) % INPUT_BUFFER_SIZE;
        buffer[buf_idx] = temp_buf[i];
        buffer_count++;
    }
}

EscapeSequence Input_Read(int fd) {
    reset_results();

    // Use a temporary buffer for the current read attempt
    UTF8Char temp_buf[INPUT_BUFFER_SIZE] = {0};
    int temp_count = 0;

    for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
        temp_buf[i] = UTF8_GetChar(fd);
        temp_count++;

        // Case 1: Timeout or multi-byte character read.
        if (temp_buf[i].length != 1) {
            int seq_idx = check_for_match(i);
            if (seq_idx != -1) {
                // A sequence was completed exactly before the timeout/multi-byte char.
                // Push the last read character (if any) to the main buffer.
                if (temp_buf[i].length > 0) {
                    push_to_buffer(&temp_buf[i], 1);
                }
                return (EscapeSequence)seq_idx;
            }
            // Incomplete sequence or not a sequence at all. Push all read chars to buffer.
            push_to_buffer(temp_buf, temp_count);
            return ESC_NONE;
        }

        // Case 2: Filter sequences based on the new character.
        filter_possible_sequences(temp_buf, i);

        if (results_count == 0) {
            // This character doesn't match any known sequence. Push all read chars to buffer.
            push_to_buffer(temp_buf, temp_count);
            return ESC_NONE;
        }
    }

    logWarn("Input buffer overflow.");
    return ESC_NONE;
}

UTF8Char Input_GetChar() {
    if (buffer_count == 0) {
        return (UTF8Char) {.bytes = {0}, .length = 0 };
    }
    buffer_count--;
    UTF8Char ch = buffer[buffer_start];
    buffer_start = (buffer_start + 1) % INPUT_BUFFER_SIZE;
    return ch;
}
