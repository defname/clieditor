#include "acutest.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

// Inkludiere die C-Datei direkt.
#include "utils/logging.c"

void test_log_output(void) {
    // stderr auf eine Pipe umleiten, um die Ausgabe abzufangen
    int pipefd[2];
    TEST_ASSERT(pipe(pipefd) == 0);

    int original_stderr = dup(STDERR_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    // Log-Nachricht schreiben
    const char* test_msg = "Dies ist ein Test";
    logInfo("Info: %s %d", test_msg, 123);

    // stderr wiederherstellen
    fflush(stderr);
    dup2(original_stderr, STDERR_FILENO);
    close(original_stderr);

    // Ausgabe aus der Pipe lesen
    char buffer[256] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    // Überprüfen, ob die Ausgabe den erwarteten Text enthält
    TEST_CHECK(strstr(buffer, "[INFO]") != NULL);
    TEST_CHECK(strstr(buffer, "Dies ist ein Test 123") != NULL);
}

void test_log_levels(void) {
    // Diese Funktion testet nicht die Ausgabe, sondern nur, dass die
    // Funktionen ohne Fehler aufgerufen werden können.
    // Ein echter Test würde wie oben die Ausgabe für jeden Level prüfen.
    TEST_CASE("Debug");
    logDebug("Debug message");

    TEST_CASE("Warn");
    logWarn("Warning message");

    TEST_CASE("Error");
    logError("Error message");

    // logFatal würde das Programm beenden, daher testen wir es hier nicht direkt.
    TEST_CHECK(1); // Platzhalter, damit der Test nicht leer ist
}

TEST_LIST = {
    { "log_output_redirection", test_log_output },
    { "log_levels_callable", test_log_levels },
    { NULL, NULL }
};