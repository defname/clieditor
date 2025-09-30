#include "acutest.h"
#include "document/textbuffer.c" // Include C file to test static functions

void test_init_tb(void) {
    TextBuffer tb;

    TB_Init(&tb);
    TEST_ASSERT(tb.cursor_pos == 0);
    TEST_ASSERT(tb.current_line != NULL);
    TEST_ASSERT(tb.current_line->text.length == 0);
    TEST_ASSERT(tb.current_line->prev == NULL);
    TEST_ASSERT(tb.current_line->next == NULL);
    TEST_ASSERT(tb.gap.text.length == 0);
    TB_Deinit(&tb);
    TEST_ASSERT(tb.current_line == NULL);
    TEST_ASSERT(tb.gap.text.length == 0);
    TEST_ASSERT(tb.gap.text.chars == NULL);
}

void test_inser_line(void) {
    TextBuffer tb;
    TB_Init(&tb);
    
    TB_InsertLineAfter(&tb);
    TEST_ASSERT(tb.current_line->next != NULL);
    TEST_ASSERT(tb.current_line->next->prev == tb.current_line);
    Line *last = tb.current_line->next;

    TB_InsertLineAfter(&tb);
    TEST_ASSERT(tb.current_line->next != NULL);
    TEST_ASSERT(tb.current_line->next->prev == tb.current_line);
    TEST_ASSERT(tb.current_line->next->next == last);
    TEST_ASSERT(last->prev == tb.current_line);

    TB_Deinit(&tb);
}

void test_insert_char(void) {
    TextBuffer tb;
    TB_Init(&tb);
    UTF8Char ch;
    ch.length = 1;
    ch.bytes[0] = 'a';
    TB_InsertChar(&tb, ch);
    TEST_ASSERT(tb.gap.text.length == 1);
    TEST_ASSERT(UTF8_Equal(tb.gap.text.chars[0], ch));

    TB_Deinit(&tb);
}

void test_merging_gap(void) {
    TextBuffer tb;

    // without overlap
    TB_Init(&tb);
    UTF8String_FromStr(&tb.current_line->text, "abcdef", 6);
    UTF8String_FromStr(&tb.gap.text, "123", 3);
    tb.cursor_pos = 3;
    TB_MergeGap(&tb);
    TEST_ASSERT(tb.current_line->text.length == 9);
    TEST_ASSERT(tb.current_line->text.chars[3].bytes[0] == '1');
    TEST_ASSERT(tb.current_line->text.chars[4].bytes[0] == '2');
    TEST_ASSERT(tb.current_line->text.chars[5].bytes[0] == '3');
    TEST_ASSERT(tb.gap.text.length == 0);
    TEST_ASSERT(tb.gap.overlap == 0);
    TB_Deinit(&tb);

    // with overlap
    TB_Init(&tb);
    UTF8String_FromStr(&tb.current_line->text, "abcdef", 6);
    UTF8String_FromStr(&tb.gap.text, "123", 3);
    tb.cursor_pos = 3;
    tb.gap.overlap = 1;
    TB_MergeGap(&tb);
    TEST_ASSERT(tb.current_line->text.length == 8);
    TEST_ASSERT(tb.current_line->text.chars[2].bytes[0] == '1');
    TEST_ASSERT(tb.current_line->text.chars[3].bytes[0] == '2');
    TEST_ASSERT(tb.current_line->text.chars[4].bytes[0] == '3');
    TEST_ASSERT(tb.gap.text.length == 0);
    TEST_ASSERT(tb.gap.overlap == 0);
    TB_Deinit(&tb);

}


TEST_LIST = {
    { "initialize/deinitialize TextBuffer", test_init_tb },
    { "inserting lines", test_inser_line },
    { "inserting chars", test_insert_char },
    { "merging gap", test_merging_gap },
    { NULL, NULL }
};
