// 1. Binde das Test-Framework ein.
// Da acutest.h im selben Verzeichnis liegt, ist kein Pfad nötig.
// WICHTIG: Dies muss vor allen anderen Includes stehen, die main() verwenden könnten.
#include "acutest.h"

#include <unistd.h>
#include <string.h>

// Inkludiere die C-Datei direkt, um auch `static` Funktionen testen zu können.
#include "common/utf8.c" // Inkludiert auch utf8.h


// --- Deine Test-Funktionen ---

void test_get_char_length(void) {
    TEST_CASE("ASCII");
    TEST_CHECK(get_char_length('A') == 1);

    TEST_CASE("2-byte sequence");
    TEST_CHECK(get_char_length(0xc3) == 2); // Example: 'Ã'

    TEST_CASE("3-byte sequence");
    TEST_CHECK(get_char_length(0xe2) == 3); // Example: '€'

    TEST_CASE("4-byte sequence");
    TEST_CHECK(get_char_length(0xf0) == 4); // Example: Emoji

    TEST_CASE("Invalid start byte");
    TEST_CHECK(get_char_length(0x80) == 0); // Continuation byte
    TEST_CHECK(get_char_length(0xff) == 0); // Invalid
}

void test_from_string(void) {
    const char* str = "H€llo"; // H=1, €=3, l=1, l=1, o=1

    TEST_CASE("GetCharFromString");
    UTF8Char h = UTF8_GetCharFromString(&str[0]);
    TEST_CHECK(h.length == 1 && h.bytes[0] == 'H');

    UTF8Char euro = UTF8_GetCharFromString(&str[1]);
    TEST_CHECK(euro.length == 3 && (unsigned char)euro.bytes[0] == 0xe2);
}

void test_equality(void) {
    UTF8Char a = {.bytes = {'A'}, .length = 1};
    UTF8Char a_copy = {.bytes = {'A'}, .length = 1};
    UTF8Char b = {.bytes = {'B'}, .length = 1};
    UTF8Char euro = {.bytes = {(char)0xe2, (char)0x82, (char)0xac}, .length = 3};
    UTF8Char invalid = { .bytes = {0}, .length = 0 };

    TEST_CHECK(UTF8_Equal(a, a_copy) == true);
    TEST_CHECK(UTF8_Equal(a, b) == false);
    TEST_CHECK(UTF8_Equal(a, euro) == false);

    TEST_CHECK(UTF8_EqualToChar(a, 'A') == true);
    TEST_CHECK(UTF8_EqualToChar(a, 'B') == false);
    TEST_CHECK(UTF8_EqualToChar(euro, 'A') == false);
    TEST_CHECK(UTF8_EqualToChar(invalid, '\0') == false);
}

void test_io_functions(void) {
    int pipefd[2];
    TEST_ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    UTF8Char euro = {.bytes = {(char)0xe2, (char)0x82, (char)0xac}, .length = 3};

    ssize_t written = UTF8_PutChar(write_fd, euro);
    TEST_CHECK(written == 3);

    close(read_fd);
    close(write_fd);
}

void test_read_zero(void) {
    int pipefd[2];
    TEST_ASSERT(pipe(pipefd) == 0);

    int read_fd = pipefd[0];
    int write_fd = pipefd[1];

    UTF8Char zero = {.bytes = {'\0'}, .length = 1};

    ssize_t written = UTF8_PutChar(write_fd, zero);
    TEST_CHECK(written == 1);

    close(read_fd);
    close(write_fd);
}

// 2. Erstelle eine Liste aller Tests, die in dieser Datei ausgeführt werden sollen.
// `acutest` generiert daraus automatisch die `main`-Funktion.
TEST_LIST = {
    { "get_char_length", test_get_char_length },
    { "from_string", test_from_string },
    { "equality", test_equality },
    { "io_functions", test_io_functions },
    { "read_zero", test_read_zero },
    // Weitere Tests hier}
    { NULL, NULL } // Markiert das Ende der Liste
};
