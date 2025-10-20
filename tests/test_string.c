#include <locale.h>
#include <string.h>

// Initialize locale for every test to ensure wcwidth and other functions work correctly with UTF-8
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "common/string.h"

void test_string_init(void) {
    String str;
    String_Init(&str);
    TEST_CHECK(str.bytes != NULL);
    TEST_CHECK(str.bytes_capacity == STRING_INITIAL_CAPACITY);
    TEST_CHECK(str.bytes_size == 0);
    TEST_CHECK(str.multibytes == NULL);
    TEST_CHECK(str.multibytes_capacity == 0);
    TEST_CHECK(str.multibytes_size == 0);
    TEST_CHECK(str.multibytes_invalid == false);
    TEST_CHECK(str.char_count == 0);
    TEST_CHECK(strcmp(str.bytes, "") == 0);
    String_Deinit(&str);

    String *str2 = String_Create();
    TEST_CHECK(str2->bytes != NULL);
    TEST_CHECK(str2->bytes_capacity == STRING_INITIAL_CAPACITY);
    TEST_CHECK(str2->bytes_size == 0);
    TEST_CHECK(str2->multibytes == NULL);
    TEST_CHECK(str2->multibytes_capacity == 0);
    TEST_CHECK(str2->multibytes_size == 0);
    TEST_CHECK(str2->multibytes_invalid == false);
    TEST_CHECK(str2->char_count == 0);
    TEST_CHECK(strcmp(str2->bytes, "") == 0);
    String_Destroy(str2);
}

typedef struct {
    const char *cstr;
    size_t length;
    size_t char_count;
} LengthTestCase;

void test_length(void) {
    const LengthTestCase test_cases[] = {
        {"abc", 3, 3},
        {"€bc", strlen("€bc"), 3},
        {"ab€\n\t", strlen("ab€\n\t"), 5}
    };
    size_t num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < num_test_cases; i++) {
        const LengthTestCase *tc = &test_cases[i];
        String str = String_FromCStr(tc->cstr, tc->length);
        TEST_CHECK(String_Length(&str) == tc->char_count);
        TEST_CHECK(str.bytes_size == tc->length);
        TEST_CHECK(strcmp(str.bytes, tc->cstr) == 0);
        String_Deinit(&str);
    }
}


typedef struct {
    const char *cstr;
    int pos;
    size_t expected_result;
} GetCharTextCase;

void test_getchar(void) {
    const GetCharTextCase test_cases[] = {
        // basic tests
        {"abc", 0, 0},
        {"abc", 1, 1},
        {"abc", 2, 2},
        // test with multibyte character
        {"€bc", 1, 3},
        // test with negative index
        {"€bc", -1, 4},
        // test with index overflow
        {"€bc", 3, 0},
        {"€bc", 4, 3},
    };
    size_t num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i=0; i<num_test_cases; i++) {
        const GetCharTextCase *tc = &test_cases[i];
        
        TEST_CASE(tc->cstr);

        String str = String_FromCStr(tc->cstr, strlen(tc->cstr));
        size_t result = String_GetChar(&str, tc->pos) - str.bytes;
        TEST_CHECK(result == tc->expected_result);
        TEST_MSG("String_GetChar(\"%s\") expected %lu got %lu", tc->cstr, tc->expected_result, result);
        String_Deinit(&str);
    }
}

void test_resize_bytes(void) {
    String str = String_Spaces(STRING_INITIAL_CAPACITY - 1);
    TEST_CHECK(str.bytes_capacity == STRING_INITIAL_CAPACITY);
    String_AddChar(&str, "a");
    TEST_CHECK(str.bytes_capacity == STRING_GROW(STRING_INITIAL_CAPACITY));
    String_Deinit(&str);
    str = String_Spaces(STRING_INITIAL_CAPACITY - 2);
    TEST_CHECK(str.bytes_capacity == STRING_INITIAL_CAPACITY);
    TEST_CHECK(String_Length(&str) == STRING_INITIAL_CAPACITY - 2);
    String_AddChar(&str, "€");  // strlen("€") == 3 => increase needed
    TEST_CHECK(str.bytes_capacity == STRING_GROW(STRING_INITIAL_CAPACITY));
    TEST_CHECK(String_Length(&str) == STRING_INITIAL_CAPACITY - 1);
    TEST_MSG("%ld", String_Length(&str));
    TEST_CHECK(str.bytes_size == STRING_INITIAL_CAPACITY + 1);
    String_Deinit(&str);
}

void test_resize_multibytes(void) {
    String str = String_FromCStr("€", strlen("€"));
    TEST_CHECK(String_Length(&str) == 1);
    TEST_CHECK(str.bytes_size == 3);
    TEST_CHECK(str.multibytes_size == 0);
    TEST_CHECK(str.multibytes_capacity == 0);
    TEST_CHECK(str.multibytes_invalid == true);

    // trigger rebuild_multibytes()
    String_GetChar(&str, 0);
    TEST_CHECK(str.multibytes_size == 1);
    TEST_CHECK(str.multibytes_capacity == STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY);
    TEST_CHECK(str.multibytes_invalid == false);
    
    for (size_t i=1; i<STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY - 1; i++) {
        String_AddChar(&str, "€");
    }
    String_AddChar(&str, "€");
    TEST_CHECK(str.multibytes_size == STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY);

    // overload multibytes list
    String_AddChar(&str, "€");
    TEST_CHECK(str.multibytes_size == STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY + 1);
    TEST_CHECK(str.multibytes_capacity == STRING_GROW(STRING_INITIAL_MULTIBYTE_OFFSETS_CAPACITY));
    TEST_CHECK(str.multibytes_invalid == false);

    String_Deinit(&str);
}

void test_append(void) {
    // Append different strings
    String str1 = String_FromCStr("abc", 3);
    String str2 = String_FromCStr("def", 3);

    String_Append(&str1, &str2);
    TEST_CHECK(strcmp(str1.bytes, "abcdef") == 0);
    TEST_CHECK(String_Length(&str1) == 6);

    String_Deinit(&str1);
    String_Deinit(&str2);

    // Append string to itself
    str1 = String_FromCStr("abc", 3);
    String_Append(&str1, &str1);
    TEST_CHECK(strcmp(str1.bytes, "abcabc") == 0);
    TEST_CHECK(String_Length(&str1) == 6);

    String_Deinit(&str1);
}

void test_misc(void) {
    String str = String_FromCStr("abcdefghi", 9);
    StringView view = String_Slice(&str, 2, 5);
    TEST_CHECK(StringView_Length(&view) == 3);
    TEST_CHECK(StringView_EqualToCStr(&view, "cde", 3));

    // test String_Set()
    String_Set(&str, String_FromCStr("xyz", 3));
    TEST_CHECK(strcmp(String_AsCStr(&str), "xyz") == 0);

    String_Deinit(&str);

    // String_Trim()
    str = String_FromCStr("   ", 3);
    String_Trim(&str);
    TEST_CHECK(strcmp(String_AsCStr(&str), "") == 0);
    String_Deinit(&str);
    str = String_FromCStr("  Foo  bar   ", 13);
    String_Trim(&str);
    TEST_CHECK(strcmp(String_AsCStr(&str), "Foo  bar") == 0);
    String_Deinit(&str);
}

void test_split(void) {
    String delimiter = String_FromCStr(",", strlen(","));
    String str = String_FromCStr("Foo,bar,€uro,", strlen("Foo,bar,€uro,"));
    StringView *list;
    ssize_t count;
    list = String_Split(&str, &delimiter, &count);
    TEST_CHECK(count == 4);
    TEST_CHECK(StringView_EqualToCStr(&list[0], "Foo", 3));
    TEST_CHECK(StringView_EqualToCStr(&list[1], "bar", 3));
    TEST_CHECK(StringView_EqualToCStr(&list[2], "€uro", strlen("€uro")));
    TEST_CHECK(StringView_EqualToCStr(&list[3], "", 0));
    free(list);
    String_Deinit(&str);

    str = String_FromCStr(",", strlen(","));
    list = String_Split(&str, &delimiter, &count);
    TEST_CHECK(count == 2);
    TEST_CHECK(StringView_EqualToCStr(&list[0], "", 0));
    TEST_CHECK(StringView_EqualToCStr(&list[1], "", 0));
    free(list);
    String_Deinit(&str);

    str = String_FromCStr("foobar", strlen("foobar"));
    list = String_Split(&str, &delimiter, &count);
    TEST_CHECK(count == 1);
    TEST_CHECK(StringView_EqualToCStr(&list[0], "foobar", 6));
    free(list);
    String_Deinit(&str);

    str = String_FromCStr("", strlen(""));
    list = String_Split(&str, &delimiter, &count);
    TEST_CHECK(count == 1);
    TEST_CHECK(StringView_EqualToCStr(&list[0], "", 0));
    free(list);
    String_Deinit(&str);

    String_Deinit(&delimiter);

    delimiter = String_FromCStr("---", strlen("---"));
    str = String_FromCStr("Foo---bar---€uro---", strlen("Foo---bar---€uro---"));
    list = String_Split(&str, &delimiter, &count);
    TEST_CHECK(count == 4);
    TEST_CHECK(StringView_EqualToCStr(&list[0], "Foo", 3));
    TEST_CHECK(StringView_EqualToCStr(&list[1], "bar", 3));
    TEST_CHECK(StringView_EqualToCStr(&list[2], "€uro", strlen("€uro")));
    TEST_CHECK(StringView_EqualToCStr(&list[3], "", 0));
    free(list);
    String_Deinit(&str);

    String_Deinit(&delimiter);
}

void test_edgecases(void) {
    String str = String_FromCStr("Foobar€", strlen("Foobar€"));
    StringView view = String_Slice(&str, 0, String_Length(&str));
    TEST_CHECK(StringView_Length(&view) == String_Length(&str));
    const char *ch = String_GetChar(&str, String_Length(&str));
    TEST_CHECK(ch == str.bytes);
    String_Deinit(&str);

    str = String_FromCStr("Foobar€", strlen("Foobar€"));
    TEST_CHECK(strcmp(String_AsCStr(&str), "Foobar€") == 0);
    String_Deinit(&str);

    str = String_FromCStr("│", strlen("│"));
    TEST_CHECK(strcmp(String_AsCStr(&str), "│") == 0);
    String_Deinit(&str);

}


TEST_LIST = {
    { "String: Initialization", test_string_init },
    { "String: Length", test_length },
    { "String: GetChar", test_getchar },
    { "String: Resize", test_resize_bytes },
    { "String: Resize multibytes", test_resize_multibytes },
    { "String: Append", test_append },
    { "String: Misc", test_misc },
    { "String: Split", test_split },
    { "String: Edge Cases", test_edgecases },
    { NULL, NULL }
};