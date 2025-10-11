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
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "../common/logging.h"

Terminal terminal;


static void update_dimensions() {
    struct winsize w;

    // STDOUT_FILENO oder STDIN_FILENO, je nachdem worauf du arbeitest
    if (ioctl(terminal.fd_out, TIOCGWINSZ, &w) == -1) {
        logFatal("Cannont get terminal dimensions.");
    }

    terminal.cols = w.ws_col;
    terminal.rows = w.ws_row;
}

void Terminal_Init(int fd_in, int fd_out, int fd_error) {
    if (!isatty(fd_in)) {
        logFatal("File descriptor is not a terminal!");
    }

    terminal.fd_in = fd_in;
    terminal.fd_out = fd_out;
    terminal.fd_error = fd_error;
    
    if (tcgetattr(fd_in, &terminal.orig_t) == -1) { // save original settings
        logFatal("Unable to get terminal configuration.");
    }

    // set NON_CANONICAL/RAW mode
    struct termios new_attr = terminal.orig_t;
    new_attr.c_cflag &= ~(ICANON | ECHO);  // deactivate canonical mode and echo
    new_attr.c_lflag &= ISIG;  // deactivate ctrl-c and ctrl-z

    new_attr.c_cc[VTIME] = 1;  // wait max 0.1s for input
    new_attr.c_cc[VMIN] = 0;  // ...

    if (tcsetattr(fd_in, TCSAFLUSH, &new_attr) == -1) {
        logFatal("Unable to set terminal configuration");
    }

    // get terminal size
    update_dimensions();
}

void Terminal_Deinit() {
    tcsetattr(terminal.fd_in, TCSAFLUSH, &terminal.orig_t);
}

void Terminal_Update() {
    update_dimensions();
}
