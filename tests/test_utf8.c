#include <locale.h>

// Initialize locale for every test to ensure wcwidth and other functions work correctly with UTF-8
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "common/utf8.h"
#include <string.h>

// Hilfsfunktion zum Vergleichen von UTF8Char-Strukturen
static bool compare_utf8_char(UTF8Char a, const unsigned char* bytes, char length) {
    if (a.length != length) return false;
    return memcmp(a.bytes, bytes, length) == 0;
}

void test_get_char_from_string(void) {
    UTF8Char ch;

    // 1-Byte-Zeichen (ASCII)
    ch = UTF8_GetCharFromString("A");
    TEST_CHECK(ch.length == 1);
    TEST_CHECK(ch.bytes[0] == 'A');

    // 2-Byte-Zeichen
    ch = UTF8_GetCharFromString("ü"); // U+00FC -> C3 BC
    unsigned char u_umlaut[] = {0xC3, 0xBC};
    TEST_CHECK(compare_utf8_char(ch, u_umlaut, 2));

    // 3-Byte-Zeichen
    ch = UTF8_GetCharFromString("€"); // U+20AC -> E2 82 AC
    unsigned char euro[] = {0xE2, 0x82, 0xAC};
    TEST_CHECK(compare_utf8_char(ch, euro, 3));

    // 4-Byte-Zeichen
    ch = UTF8_GetCharFromString("😊"); // U+1F60A -> F0 9F 98 8A
    unsigned char smiley[] = {0xF0, 0x9F, 0x98, 0x8A};
    TEST_CHECK(compare_utf8_char(ch, smiley, 4));

    // Ungültige Sequenz (sollte als ungültig erkannt werden)
    ch = UTF8_GetCharFromString("\x80"); // Fortsetzungsbyte am Anfang
    TEST_CHECK(UTF8_Equal(ch, utf8_invalid));
}

void test_equality(void) {
    UTF8Char ch_A1 = UTF8_GetCharFromString("A");
    UTF8Char ch_A2 = UTF8_GetCharFromString("A");
    UTF8Char ch_B = UTF8_GetCharFromString("B");
    UTF8Char ch_ue = UTF8_GetCharFromString("ü");

    // Gleichheit
    TEST_CHECK(UTF8_Equal(ch_A1, ch_A2));
    TEST_CHECK(UTF8_Equal(ch_ue, UTF8_GetCharFromString("ü")));

    // Ungleichheit
    TEST_CHECK(!UTF8_Equal(ch_A1, ch_B));
    TEST_CHECK(!UTF8_Equal(ch_A1, ch_ue));

    // Gleichheit mit char
    TEST_CHECK(UTF8_EqualToChar(ch_A1, 'A'));
    TEST_CHECK(!UTF8_EqualToChar(ch_A1, 'B'));
    TEST_CHECK(!UTF8_EqualToChar(ch_ue, 'u')); // 'ü' ist nicht 'u'
}

void test_codepoint_conversion(void) {
    TEST_CHECK(UTF8_ToCodepoint(UTF8_GetCharFromString("A")) == 0x41);
    TEST_CHECK(UTF8_ToCodepoint(UTF8_GetCharFromString("ü")) == 0xFC);
    TEST_CHECK(UTF8_ToCodepoint(UTF8_GetCharFromString("€")) == 0x20AC);
    TEST_CHECK(UTF8_ToCodepoint(UTF8_GetCharFromString("😊")) == 0x1F60A);
    TEST_CHECK(UTF8_ToCodepoint(utf8_invalid) == INVALID_CODEPOINT);
}

void test_char_properties(void) {
    UTF8Char ch_A = UTF8_GetCharFromString("A");
    UTF8Char ch_ue = UTF8_GetCharFromString("ü");
    UTF8Char ch_tab = UTF8_GetCharFromString("\t");
    UTF8Char ch_space = UTF8_GetCharFromString(" ");
    UTF8Char ch_emoji = UTF8_GetCharFromString("😊");
    UTF8Char ch_nul = UTF8_GetCharFromString("\0");

    // IsASCII / AsASCII
    TEST_CHECK(UTF8_IsASCII(ch_A));
    TEST_CHECK(!UTF8_IsASCII(ch_ue));
    TEST_CHECK(UTF8_AsASCII(ch_A) == 'A');
    TEST_CHECK(UTF8_AsASCII(ch_ue) == '\0'); // Nicht-ASCII gibt 0 zurück

    // IsSpace
    TEST_CHECK(UTF8_IsSpace(ch_space));
    TEST_CHECK(UTF8_IsSpace(ch_tab));
    TEST_CHECK(!UTF8_IsSpace(ch_A));

    // IsPrintable
    TEST_CHECK(UTF8_IsPrintable(ch_A));
    TEST_CHECK(UTF8_IsPrintable(ch_ue));
    TEST_CHECK(!UTF8_IsPrintable(ch_tab)); // Steuerzeichen sind nicht druckbar
    TEST_CHECK(!UTF8_IsPrintable(ch_nul));

    // GetWidth (Annahme: Emoji hat Breite 2, Rest 1)
    TEST_CHECK(UTF8_GetWidth(ch_A) == 1);
    TEST_CHECK(UTF8_GetWidth(ch_ue) == 1);
    TEST_CHECK(UTF8_GetWidth(ch_emoji) == 2);
    TEST_CHECK(UTF8_GetWidth(ch_tab) == 1); // Breite von Steuerzeichen ist oft 1
}


TEST_LIST = {
    { "UTF8_GetCharFromString", test_get_char_from_string },
    { "UTF8_Equal / UTF8_EqualToChar", test_equality },
    { "UTF8_ToCodepoint", test_codepoint_conversion },
    { "UTF8 Character Properties", test_char_properties },
    { NULL, NULL }
};