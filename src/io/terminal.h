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
void Terminal_Deinit();
void Terminal_Update();  // Updates the dimensions, should be called when signal SIGWINCH arrives

#endif