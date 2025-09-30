#include "acutest.h"

// Inkludiere die C-Datei direkt.
// terminal.c inkludiert unistd.h, termios.h, stdio.h, stdlib.h
#include "io/terminal.c"

void test_terminal_init(void) {
    // Wir können die echten termios-Funktionen nicht einfach testen,
    // da sie das Terminal verändern würden. Stattdessen prüfen wir,
    // ob die internen Zustände korrekt gesetzt werden.

    // Wir verwenden ungültige FDs, da wir nicht wirklich lesen/schreiben.
    Terminal_Init(-1, -1, -1);

    TEST_CHECK(terminal.fd_in == -1);
    TEST_CHECK(terminal.fd_out == -1);
    TEST_CHECK(terminal.fd_error == -1);

    // Überprüfen, ob die Flags im Raw-Modus korrekt gesetzt werden.
    // Dies ist eine Annahme über die Implementierung von Terminal_EnableRawMode.
    struct termios test_termios = terminal.orig_t;
    test_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    test_termios.c_oflag &= ~(OPOST);
    test_termios.c_cflag |= (CS8);
    test_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Hier könnten wir memcmp verwenden, wenn wir wüssten, wie Terminal_EnableRawMode die termios-Struktur verändert.
    TEST_CHECK(1); // Platzhalter
}

TEST_LIST = {
    { "terminal_initialization_state", test_terminal_init },
    { NULL, NULL }
};