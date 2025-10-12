#include <locale.h>
#include <string.h>

// Initialize locale for every test to ensure wcwidth and other functions work correctly with UTF-8
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "common/utf8string.h"
#include "common/utf8.h"

// Helper to compare a UTF8String with a C-string
static void check_string_equals(const UTF8String *s, const char *expected_cstr) {
    char *actual_cstr = UTF8String_ToStr(s);
    TEST_CHECK(strcmp(actual_cstr, expected_cstr) == 0);
    TEST_MSG("Expected: '%s', Got: '%s'", expected_cstr, actual_cstr);
    free(actual_cstr);
}

void test_creation_and_conversion(void) {
    // Test 1: Create an empty string
    UTF8String *s1 = UTF8String_Create();
    TEST_ASSERT(s1 != NULL);
    TEST_CHECK(UTF8String_Length(s1) == 0);
    check_string_equals(s1, "");
    UTF8String_Destroy(s1);

    // Test 2: Create from a C-string
    UTF8String *s2 = UTF8String_Create();
    const char *test_str = "Grüße 😊"; // G,r,ü,ß,e, ,😊 (7 chars)
    UTF8String_FromStr(s2, test_str, strlen(test_str));
    TEST_CHECK(UTF8String_Length(s2) == 7);
    check_string_equals(s2, test_str);
    UTF8String_Destroy(s2);

    // Test 3: NULL safety for Destroy
    UTF8String_Destroy(NULL); // Should not crash
}

void test_equality(void) {
    UTF8String *s1 = UTF8String_Create();
    UTF8String *s2 = UTF8String_Create();
    UTF8String *s3 = UTF8String_Create();

    UTF8String_FromStr(s1, "Hello", 5);
    char *s1_c = UTF8String_ToStr(s1);
    TEST_ASSERT(strcmp(s1_c, "Hello") == 0);
    TEST_ASSERT(UTF8String_EqualStr(s1, s1_c));
    TEST_ASSERT(!UTF8String_EqualStr(s1, "Hello "));
    TEST_ASSERT(!UTF8String_EqualStr(s1, "Hellö"));
    TEST_ASSERT(!UTF8String_EqualStr(s1, "Helko"));
    free(s1_c);

    UTF8String_FromStr(s1, "ü", 1);
    s1_c = UTF8String_ToStr(s1);
    TEST_ASSERT(strcmp(s1_c, "ü") == 0);
    TEST_ASSERT(UTF8String_EqualStr(s1, "ü"));
    char *test = UTF8String_ToStr(s1);
    free(test);
    free(s1_c);

    UTF8String_FromStr(s1, "Hello\nWorld\0", 12);
    s1_c = UTF8String_ToStr(s1);
    TEST_ASSERT(strcmp(s1_c, "Hello\nWorld") == 0);

    UTF8String_FromStr(s2, "Hello\nWorld", 11);
    TEST_ASSERT(UTF8String_Equal(s1, s2));
    
    free(s1_c);
    UTF8String_Destroy(s1);
    UTF8String_Destroy(s2);
    UTF8String_Destroy(s3);
}

void test_add_char_and_capacity(void) {
    UTF8String *s = UTF8String_Create();
    TEST_ASSERT(s != NULL);
    TEST_CHECK(s->capacity >= UTF8STRING_INITIAL_CAPACITY);

    // Add a few characters
    UTF8String_AddChar(s, UTF8_GetCharFromString("A"));
    UTF8String_AddChar(s, UTF8_GetCharFromString("ü"));
    TEST_CHECK(UTF8String_Length(s) == 2);
    check_string_equals(s, "Aü");

    // Force capacity increase
    size_t initial_cap = s->capacity;
    for (size_t i = s->length; i < initial_cap + 5; ++i) {
        UTF8String_AddChar(s, UTF8_GetCharFromString("x"));
    }
    TEST_CHECK(s->capacity > initial_cap);
    TEST_CHECK(UTF8String_Length(s) == initial_cap + 5);

    UTF8String_Destroy(s);
}

void test_copy_and_shorten(void) {
    UTF8String *src = UTF8String_Create();
    UTF8String_FromStr(src, "Hello World", strlen("Hello World"));

    UTF8String *dest = UTF8String_Create();
    UTF8String_Copy(dest, src);

    TEST_CHECK(UTF8String_Length(dest) == 11);
    check_string_equals(dest, "Hello World");

    // Check that they are independent
    UTF8String_AddChar(src, UTF8_GetCharFromString("!"));
    check_string_equals(src, "Hello World!");
    check_string_equals(dest, "Hello World"); // dest should be unchanged

    // Test Shorten
    UTF8String_Shorten(dest, 5);
    TEST_CHECK(UTF8String_Length(dest) == 5);
    check_string_equals(dest, "Hello");

    // Shortening to a longer or equal length should do nothing
    UTF8String_Shorten(dest, 5);
    TEST_CHECK(UTF8String_Length(dest) == 5);
    UTF8String_Shorten(dest, 100);
    TEST_CHECK(UTF8String_Length(dest) == 5);

    UTF8String_Destroy(src);
    UTF8String_Destroy(dest);
}

void test_concat_and_repeat(void) {
    UTF8String *s1 = UTF8String_Create();
    UTF8String_FromStr(s1, "Hallo ", 6);

    UTF8String *s2 = UTF8String_Create();
    UTF8String_FromStr(s2, "Welt! 👋", strlen("Welt! 👋"));

    UTF8String_Concat(s1, s2);
    TEST_CHECK(UTF8String_Length(s1) == 6 + 7); // "Hallo " + "Welt!" + "👋"
    check_string_equals(s1, "Hallo Welt! 👋");

    // Test Repeat
    UTF8String *s3 = UTF8String_Create();
    UTF8String_FromStr(s3, "Abc", 3);
    UTF8String_Repeat(s3, 3);
    TEST_CHECK(UTF8String_Length(s3) == 9);
    check_string_equals(s3, "AbcAbcAbc");

    // Repeat 0 and 1
    UTF8String_Repeat(s3, 1);
    check_string_equals(s3, "AbcAbcAbc");
    UTF8String_Repeat(s3, 0);
    TEST_CHECK(UTF8String_Length(s3) == 0);
    check_string_equals(s3, "");

    UTF8String_Destroy(s1);
    UTF8String_Destroy(s2);
    UTF8String_Destroy(s3);
}

void test_split(void) {
    UTF8String *s = UTF8String_Create();
    UTF8String_FromStr(s, "012ü45€789", strlen("012ü45€789")); // 10 chars

    UTF8String *a = UTF8String_Create();
    UTF8String *b = UTF8String_Create();

    // Split in the middle
    UTF8String_Split(s, a, b, 4);
    TEST_CHECK(UTF8String_Length(a) == 4);
    check_string_equals(a, "012ü");
    TEST_CHECK(UTF8String_Length(b) == 6);
    check_string_equals(b, "45€789");

    // Split at the beginning
    UTF8String_Split(s, a, b, 0);
    TEST_CHECK(UTF8String_Length(a) == 0);
    check_string_equals(b, "012ü45€789");

    // Split at the end
    UTF8String_Split(s, a, b, 10);
    check_string_equals(a, "012ü45€789");
    TEST_CHECK(UTF8String_Length(b) == 0);

    UTF8String_Destroy(s);
    UTF8String_Destroy(a);
    UTF8String_Destroy(b);
}

void test_string_width(void) {
    // Test 1: Empty string
    UTF8String *s1 = UTF8String_Create();
    TEST_CHECK(UTF8String_Width(s1) == 0);
    UTF8String_Destroy(s1);

    // Test 2: Simple ASCII
    UTF8String *s2 = UTF8String_Create();
    UTF8String_FromStr(s2, "Hello", 5);
    TEST_CHECK(UTF8String_Width(s2) == 5);
    UTF8String_Destroy(s2);

    // Test 3: String with multi-byte chars (width 1)
    UTF8String *s3 = UTF8String_Create();
    UTF8String_FromStr(s3, "Grüße", strlen("Grüße")); // 5 chars, should be 5 width
    TEST_CHECK(UTF8String_Width(s3) == 5);

    // Test 4: String with wide chars (width 2) and SubstringWidth
    UTF8String_FromStr(s3, "Hi 😊", strlen("Hi 😊")); // H,i, ,😊 (4 chars) -> width 1+1+1+2 = 5
    TEST_CHECK(UTF8String_Width(s3) == 5);
    TEST_MSG("Expected width 5 for 'Hi 😊', got %d", UTF8String_Width(s3));

    // Test SubstringWidth
    TEST_CHECK(UTF8String_SubstringWidth(s3, 0, 2) == 2); // "Hi"
    TEST_CHECK(UTF8String_SubstringWidth(s3, 3, 4) == 2); // "😊"
    TEST_CHECK(UTF8String_SubstringWidth(s3, 0, 4) == 5); // Full string
    TEST_CHECK(UTF8String_SubstringWidth(s3, 2, 100) == 3); // " 😊" and out of bounds
    UTF8String_Destroy(s3);
}

void test_substring(void) {
    UTF8String *s = UTF8String_Create();
    UTF8String *a = UTF8String_Create();

    UTF8String_FromStr(s, "Hello World", 11);

    UTF8String_SubString(s, a, 0, 5);
    TEST_CHECK(UTF8String_EqualStr(a, "Hello"));

    UTF8String_SubString(s, a, 6, 5);
    TEST_CHECK(UTF8String_EqualStr(a, "World"));

    UTF8String_SubString(s, a, 6, 10);
    TEST_CHECK(UTF8String_EqualStr(a, "World"));
    

    UTF8String_SubString(s, a, 6, 3);
    TEST_CHECK(UTF8String_EqualStr(a, "Wor"));

    UTF8String_Destroy(s);
    UTF8String_Destroy(a);
}

TEST_LIST = {
    { "UTF8String: Creation and Conversion", test_creation_and_conversion },
    { "UTF8String: Equality", test_equality },
    { "UTF8String: AddChar and Capacity", test_add_char_and_capacity },
    { "UTF8String: Copy and Shorten", test_copy_and_shorten },
    { "UTF8String: Concat and Repeat", test_concat_and_repeat },
    { "UTF8String: Split", test_split },
    { "UTF8String: Width", test_string_width },
    { "UTF8String: Substring", test_substring },
    { NULL, NULL }
};