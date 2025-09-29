#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

int main(void) {
    struct termios oldt, newt;
    char ch;

    // Terminal-Einstellungen sichern
    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        perror("tcgetattr");
        return 1;
    }
    newt = oldt;

    // Raw Mode aktivieren
    newt.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1) {
        perror("tcsetattr");
        return 1;
    }

    printf("Press any key to exit...\n");
    fflush(stdout);

    // select() blockiert, bis Eingabe wirklich verfügbar ist
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    if (select(STDIN_FILENO + 1, &set, NULL, NULL, NULL) == -1) {
        perror("select");
    } else {
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            printf("You pressed: %c (code %d)\n", ch, (unsigned char)ch);
        }
    }

    // Terminal zurücksetzen
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1) {
        perror("tcsetattr restore");
        return 1;
    }

    return 0;
}
