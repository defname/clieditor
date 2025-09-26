/**
 * terminal.h
 * Responsable for initializing the terminal in non-canonical/raw mode and retrieving the dimensions.
 * Populates the global terminal object.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

typedef struct _Terminal {
    // file descriptors
    int fd_in;
    int fd_out;
    int fd_error;

    // dimensions (automatically updated)
    int cols;
    int rows;

    // original terminal settings
    struct termios orig_t;
} Terminal;

extern Terminal terminal;

void Terminal_Init(int fd_in, int fd_out, int fd_error);
void Terminal_Finish();
void Terminal_Update();  // Updates the dimensions, should be called when signal SIGWINCH arrives

#endif