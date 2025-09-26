#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "core/terminal.h"
#include "core/screen.h"
#include "core/utf8.h"

void finish() {
    Screen_Finish();Screen_Clear();
    Terminal_Finish();
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    atexit(finish);  // make sure original settings are restored

    Terminal_Init(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    Screen_Init();

    // Initialer Screen-Aufbau
    Screen_Clear();
    Screen_Draw();
    Screen_HideCursor();

    while (1) {
        UTF8Char c;
        // Blockiert, bis eine Taste gedrückt wird. Verbraucht keine CPU.
        if ((c = UTF8_GetChar(terminal.fd_in)).length == 0) {
            break; // Fehler oder Signal
        }

        // Programm mit 'q' beenden
        if (UTF8_EqualToChar(c, 'q')) {
            break;
        }

        // Back-Buffer aktualisieren
        char buffer[64];
        char char_bytes[5] = {0};
        memcpy(char_bytes, c.bytes, c.length);
        snprintf(buffer, sizeof(buffer), "Gedrückt: %s | Terminal: %dx%d", char_bytes, terminal.cols, terminal.rows);
        UTF8Char buffer_utf8[64];
        size_t l = UTF8_FromString(buffer_utf8, 64, buffer);
        Screen_MoveCursor(0, 0);
        Screen_Write(buffer_utf8, l);

        // Änderungen auf den Bildschirm zeichnen
        Screen_Draw();
    }
}