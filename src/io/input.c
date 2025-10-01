#include "input.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "common/utf8.h"
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


//--------- Circular Buffer ----------
#define BUFFER_MAX INPUT_BUFFER_SIZE


//-------------- Sequences -----------------
// Flexible Verwaltung von Escape-Sequenzen.
// Um eine neue Sequenz hinzuzufügen, einfach hier einen Eintrag ergänzen.
typedef struct _EscapeSequenceMapping {
    const char *sequence;
    EscapeSequence code;
} EscapeSequenceMapping;

#define MAX_ESCAPE_SEQUENCES 20
#define MAX_SEQUENCE_LEN    6

static const EscapeSequenceMapping escape_mappings[] = {
    {"\e[A", ESC_CURSOR_UP},
    {"\e[B", ESC_CURSOR_DOWN},
    {"\e[C", ESC_CURSOR_RIGHT},
    {"\e[D", ESC_CURSOR_LEFT},
    {"\e[H", ESC_HOME},
    {"\e[F", ESC_END},
    {"\e[5~", ESC_PAGE_UP},
    {"\e[6~", ESC_PAGE_DOWN},
    {"\e[3~", ESC_DELETE},
    {"\e[A;2", ESC_SHIFT_CURSOR_UP},
    {"\e[B;2", ESC_SHIFT_CURSOR_DOWN},
    {"\e[C;2", ESC_SHIFT_CURSOR_RIGHT},
    {"\e[D;2", ESC_SHIFT_CURSOR_LEFT},
    {"\e[5;2~", ESC_SHIFT_PAGE_UP},
    {"\e[6;2~", ESC_SHIFT_PAGE_DOWN},
};
static const size_t num_escape_mappings = sizeof(escape_mappings) / sizeof(escape_mappings[0]);


// main input buffer
Buffer buffer;

void Input_Init() {
    Buffer_Init(&buffer, BUFFER_MAX, sizeof(char));
}

void Input_Deinit() {
    Buffer_Deinit(&buffer);
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

static void push_to_buffer(const unsigned char *s, size_t n) {
    if (buffer.count + n > BUFFER_MAX) {
        logError("Input buffer overflow on push. Sequence dropped.");
        return;
    }
    for (size_t i=0; i<n; i++) {
        Buffer_Enqueue(&buffer, &s[i]);
    }
}

// initialize results with all indices of escape_mappings and return the number of entries
size_t init_seq_results(size_t *results, size_t capacity) {
    if (capacity < num_escape_mappings) {
        logFatal("Results list too small.");
    }
    for (size_t i=0; i<num_escape_mappings; i++) {
        results[i] = i;
    }
    return num_escape_mappings;
}

// filter results. discard all entries that differs from c at position idx
size_t filter_seq_results(size_t *results, size_t count, size_t idx, unsigned char c) {
    int new_count = 0;
    for (size_t i=0; i<count; i++) {
        const char *seq = escape_mappings[results[i]].sequence;
        if (seq[idx] == c) {
            results[new_count] = results[i];
            new_count++;
        }
    }
    return new_count;
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
// the read sequence is written to seq, with max size of capacity
// the number of bytes read is stored in seq_count.
// the function returns true if a valid escape sequence was read, false otherwise
bool read_escape_sequence(int fd, unsigned char *seq, size_t capacity, size_t *seq_count) {
    seq[0] = '\e';
    *seq_count = 1;

    ssize_t bytes_read = read_with_timeout(fd, &seq[1], 50);
    if (bytes_read != 1) {
        return false;
    }
    (*seq_count)++;  // bytes_read == 1
    if (seq[1] != '[') {
        return false;
    }

    // so far we read '\e' in the calling function Input_Read() and '['
    // so
    // tmp_buf == { '\e', '[' }
    // tmp_buf_count == 2

    enum { PARAMS, INTER, FINAL } state = PARAMS;

    while ((*seq_count)<capacity) {
        unsigned char ch;
        // read byte from input
        bytes_read = read_with_timeout(fd, &ch, 100);
        if (bytes_read != 1) {
            // Timeout, die Sequenz ist unvollständig
            return false;
        }
        if (state == PARAMS) {
            if (is_param_byte(ch)) {
                seq[(*seq_count)++] = ch;
                continue;
            }
            state++;
        }
        if (state == INTER) {
            if (is_intermediate_byte(ch)) {
                seq[(*seq_count)++] = ch;
                continue;
            }
            state++;
        }
        if (state == FINAL) {
            if (is_final_byte(ch)) {
                seq[(*seq_count)++] = ch;
                return true;
            }
            seq[(*seq_count)++] = ch;
            return false;
        }
    }

    logError("Read sequence exceeded capacity.");
    return false;
}

EscapeSequence find_sequence(const unsigned char *seq, size_t n) {
    for (size_t i=0; i<num_escape_mappings; i++) {
        // Check for an exact match: the length must be the same AND the content must be the same.
        if (strlen(escape_mappings[i].sequence) == n &&
            strncmp(escape_mappings[i].sequence, (const char*)seq, n) == 0) {
            return escape_mappings[i].code;
        }
    }
    return ESC_NONE;
}

EscapeSequence Input_Read() {
    if (buffer.count >= INPUT_BUFFER_SIZE) {
        return ESC_NONE;
    }

    int fd = terminal.fd_in;

    unsigned char c;
    ssize_t byted_read = read(fd, &c, 1);
    if (byted_read != 1) {
        return ESC_NONE;
    }

    if (c != '\e') {
        Buffer_Enqueue(&buffer, &c);
        return ESC_NONE;
    }

    unsigned char seq_buffer[MAX_SEQUENCE_LEN];
    size_t len = 0;
    bool is_seq = read_escape_sequence(fd, seq_buffer, MAX_SEQUENCE_LEN, &len);

    if (is_seq) {
        EscapeSequence seq = find_sequence(seq_buffer, len);
        if (seq != ESC_NONE) {
            return seq;
        }
        logDebug("Unknown escape sequence: %.*s", (int)len, (const char*)seq_buffer);
        return ESC_NONE;
    }
    push_to_buffer(seq_buffer + 1, len-1);
    if (len > 1) {
        return ESC_NONE;
    }
    return ESC_ESCAPE;
}

UTF8Char Input_GetChar() {
    return UTF8_ReadCharFromBuf(&buffer);
}
