// setlocale to recognize wide characters correctly
#include <locale.h>
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "common/utf8_helper.h"

void test_calc_width(void) {
    // Null character should have zero width
    TEST_CHECK(utf8_calc_width('\0') == 0);

    // Standard ASCII characters should have a width of 1
    TEST_CHECK(utf8_calc_width('a') == 1);

    // Wide characters (like many emojis and CJK characters) should have a width of 2
    TEST_CHECK(utf8_calc_width(0x1F60A) == 2); // 😊 Smiling Face with Smiling Eyes
    TEST_CHECK(utf8_calc_width(0x4E2D) == 2);  // 中 (Chinese character for "middle")
}

void test_to_codepoint(void) {
    TEST_CHECK(utf8_to_codepoint("A") == 0x41);
    TEST_CHECK(utf8_to_codepoint("ü") == 0xFC);
    TEST_CHECK(utf8_to_codepoint("€") == 0x20AC);
    TEST_CHECK(utf8_to_codepoint("😊") == 0x1F60A);
}

TEST_LIST = {
    { "utf8_calc_width", test_calc_width },
    { "utf8_to_codepoint", test_to_codepoint },
    { NULL, NULL }
};
