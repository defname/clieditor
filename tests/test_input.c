#include "acutest.h"
#include "core/input.c" // Include C file to test static functions

void test_init_seq_results(void) {
    size_t results[MAX_ESCAPE_SEQUENCES];
    size_t count = init_seq_results(results, MAX_ESCAPE_SEQUENCES);

    TEST_CHECK(count == num_escape_mappings);
    for (size_t i = 0; i < count; i++) {
        TEST_CHECK(results[i] == i);
    }
}

void test_filter_seq_results(void) {
    size_t results[MAX_ESCAPE_SEQUENCES];
    size_t count;

    // --- Test 1: Filter for a unique sequence ---
    count = init_seq_results(results, MAX_ESCAPE_SEQUENCES);

    // Filter for '\e[A' (Cursor Up)
    count = filter_seq_results(results, count, 1, '[');
    count = filter_seq_results(results, count, 2, 'A');

    TEST_CHECK(count == 1);
    TEST_CHECK(results[0] == 0); // Index of {"\e[A", ESC_CURSOR_UP}
    TEST_MSG("Expected to find only ESC_CURSOR_UP");

    // --- Test 2: Filter for a non-existent sequence ---
    count = init_seq_results(results, MAX_ESCAPE_SEQUENCES);
    count = filter_seq_results(results, count, 1, '[');
    count = filter_seq_results(results, count, 2, 'Z'); // 'Z' is not a known final byte

    TEST_CHECK(count == 0);
    TEST_MSG("Expected to find no matching sequence for '[Z'");

    // --- Test 3: Filter for a common prefix ---
    count = init_seq_results(results, MAX_ESCAPE_SEQUENCES);
    count = filter_seq_results(results, count, 1, '[');

    // All sequences except ESC_ESCAPE start with '\e['
    size_t expected_count = 0;
    for(size_t i=0; i<num_escape_mappings; i++) {
        if(strlen(escape_mappings[i].sequence) > 1 && escape_mappings[i].sequence[1] == '[') {
            expected_count++;
        }
    }
    TEST_CHECK(count == expected_count);
    TEST_MSG("Expected to find all sequences starting with '['");
}

void test_byte_classifiers(void) {
    // is_param_byte: 0x30 - 0x3F
    TEST_CHECK(is_param_byte('0') == true);
    TEST_CHECK(is_param_byte('9') == true);
    TEST_CHECK(is_param_byte(';') == true);
    TEST_CHECK(is_param_byte('?') == true);
    TEST_CHECK(is_param_byte(' ') == false);
    TEST_CHECK(is_param_byte('A') == false);

    // is_intermediate_byte: 0x20 - 0x2F
    TEST_CHECK(is_intermediate_byte(' ') == true);
    TEST_CHECK(is_intermediate_byte('/') == true);
    TEST_CHECK(is_intermediate_byte('!') == true);
    TEST_CHECK(is_intermediate_byte('0') == false);
    TEST_CHECK(is_intermediate_byte('A') == false);

    // is_final_byte: 0x40 - 0x7E
    TEST_CHECK(is_final_byte('@') == true);
    TEST_CHECK(is_final_byte('~') == true);
    TEST_CHECK(is_final_byte('A') == true);
    TEST_CHECK(is_final_byte(' ') == false);
    TEST_CHECK(is_final_byte(0x1F) == false);
}

void test_find_sequence(void) {
    // Test finding a known sequence
    unsigned char seq1[] = "\e[A";
    EscapeSequence code1 = find_sequence(seq1, 3);
    TEST_CHECK(code1 == ESC_CURSOR_UP);

    // Test with a sequence that is a prefix of another
    unsigned char seq2[] = "\e[";
    EscapeSequence code2 = find_sequence(seq2, 2);
    TEST_CHECK(code2 == ESC_NONE); // Should not match, as it's incomplete

    // Test with an unknown sequence
    unsigned char seq3[] = "\e[Z";
    EscapeSequence code3 = find_sequence(seq3, 3);
    TEST_CHECK(code3 == ESC_NONE);

    // Test with a sequence containing numbers
    unsigned char seq4[] = "\e[5~";
    EscapeSequence code4 = find_sequence(seq4, 4);
    TEST_CHECK(code4 == ESC_PAGE_UP);

    // Test with a sequence that is too long
    unsigned char seq5[] = "\e[ABC";
    EscapeSequence code5 = find_sequence(seq5, 4);
    TEST_CHECK(code5 == ESC_NONE);
}

void test_read_escape_sequence(void) {
    int fds[2];
    unsigned char seq_buf[MAX_SEQUENCE_LEN];
    size_t seq_len;
    bool result;

    // Helper macro for setup and teardown
    #define PREPARE_TEST(input_str) \
        TEST_ASSERT(pipe(fds) == 0); \
        write(fds[1], input_str, strlen(input_str)); \
        memset(seq_buf, 0, sizeof(seq_buf)); \
        seq_len = 0

    #define CLEANUP_TEST() \
        close(fds[0]); \
        close(fds[1])

    // --- Test 1: Simple valid sequence ---
    PREPARE_TEST("[A");
    result = read_escape_sequence(fds[0], seq_buf, MAX_SEQUENCE_LEN, &seq_len);
    TEST_CHECK(result == true);
    TEST_CHECK(seq_len == 3);
    TEST_CHECK(strncmp((char*)seq_buf, "\e[A", 3) == 0);
    TEST_MSG("Failed to read simple sequence '[A'");
    CLEANUP_TEST();

    // --- Test 2: Valid sequence with parameter ---
    PREPARE_TEST("[5~");
    result = read_escape_sequence(fds[0], seq_buf, MAX_SEQUENCE_LEN, &seq_len);
    TEST_CHECK(result == true);
    TEST_CHECK(seq_len == 4);
    TEST_CHECK(strncmp((char*)seq_buf, "\e[5~", 4) == 0);
    TEST_MSG("Failed to read sequence with parameter '[5~'");
    CLEANUP_TEST();

    // --- Test 3: Incomplete sequence (timeout) ---
    PREPARE_TEST("[");
    result = read_escape_sequence(fds[0], seq_buf, MAX_SEQUENCE_LEN, &seq_len);
    TEST_CHECK(result == false);
    TEST_CHECK(seq_len == 2); // Should have read '\e' (pre-filled) and '['
    TEST_CHECK(strncmp((char*)seq_buf, "\e[", 2) == 0);
    TEST_MSG("Failed to handle incomplete sequence '['");
    CLEANUP_TEST();

    // --- Test 4: Invalid final byte ---
    PREPARE_TEST("[ "); // Space is an intermediate, not a final byte
    result = read_escape_sequence(fds[0], seq_buf, MAX_SEQUENCE_LEN, &seq_len);
    TEST_CHECK(result == false);
    TEST_CHECK(seq_len == 3);
    TEST_CHECK(strncmp((char*)seq_buf, "\e[ ", 3) == 0);
    TEST_MSG("Failed to handle invalid final byte in '[ '");
    CLEANUP_TEST();

    #undef PREPARE_TEST
    #undef CLEANUP_TEST
}


TEST_LIST = {
    { "init_seq_results", test_init_seq_results },
    { "filter_seq_results", test_filter_seq_results },
    { "byte_classifiers", test_byte_classifiers },
    { "find_sequence", test_find_sequence },
    { "read_escape_sequence", test_read_escape_sequence },
    { NULL, NULL }
};
