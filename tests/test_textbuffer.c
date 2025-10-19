#include <locale.h>
#include <string.h>

// Initialize locale for every test to ensure wcwidth and other functions work correctly with UTF-8
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "document/textbuffer.h"
#include "document/line.h"
#include "common/string.h"

// Helper to compare a UTF8String with a C-string
static void check_string_equals(const String *s, const char *expected_cstr) {
    TEST_CHECK(strcmp(String_AsCStr(s), expected_cstr) == 0);
    TEST_MSG("Expected: '%s', Got: '%s'", expected_cstr, String_AsCStr(s));
}

static Line *create_line(const char *text) {
    Line *line = Line_Create();
    String_Set(&line->text, String_FromCStr(text, strlen(text)));
    return line;
}

void test_textbuffer_init_deinit(void) {
    TextBuffer tb;
    TextBuffer_Init(&tb);

    TEST_CHECK(tb.current_line != NULL);
    TEST_CHECK(tb.current_line->prev == NULL);
    TEST_CHECK(tb.current_line->next == NULL);
    TEST_CHECK(String_Length(&tb.current_line->text) == 0);
    TEST_CHECK(tb.line_count == 1);
    TEST_CHECK(tb.gap.position == 0);
    TEST_CHECK(tb.gap.overlap == 0);
    TEST_CHECK(String_Length(&tb.gap.text) == 0);

    // ReInit should clear and re-initialize
    String_Set(&tb.current_line->text, String_FromCStr("some text", 9));
    TextBuffer_ReInit(&tb);
    TEST_CHECK(tb.current_line != NULL);
    TEST_CHECK(String_Length(&tb.current_line->text) == 0);
    TEST_CHECK(tb.line_count == 1);

    TextBuffer_Deinit(&tb);
}

void test_textbuffer_merge_gap_insert(void) {
    TextBuffer tb;
    TextBuffer_Init(&tb);

    // Setup based on header documentation example 1: Insert
    const char* initial_text = "I like Terminals.";
    String_Set(&tb.current_line->text, String_FromCStr(initial_text, strlen(initial_text)));

    tb.gap.position = 2; // After "I "
    tb.gap.overlap = 0;
    String_Set(&tb.gap.text, String_FromCStr("really ", 7));

    TextBuffer_MergeGap(&tb);

    // Verify the result
    check_string_equals(&tb.current_line->text, "I really like Terminals.");
    TEST_CHECK(tb.gap.position == 2 + 7); // New position is after the inserted text
    TEST_CHECK(tb.gap.overlap == 0);
    TEST_CHECK(String_Length(&tb.gap.text) == 0);

    TextBuffer_Deinit(&tb);
}

void test_textbuffer_merge_gap_overwrite(void) {
    TextBuffer tb;
    TextBuffer_Init(&tb);

    // Setup based on header documentation example 2: Overwrite
    const char* initial_text = "I like Terminals.";
    String_Set(&tb.current_line->text, String_FromCStr(initial_text, strlen(initial_text)));

    tb.gap.position = 6; // At the end of " like"
    tb.gap.overlap = 4;  // Overlap "like"
    String_Set(&tb.gap.text, String_FromCStr("love", 4));

    TextBuffer_MergeGap(&tb);

    // Verify the result
    check_string_equals(&tb.current_line->text, "I love Terminals.");
    TEST_CHECK(tb.gap.position == 6 + 4 - 4); // New position is at the end of "love"
    TEST_CHECK(tb.gap.overlap == 0);
    TEST_CHECK(String_Length(&tb.gap.text) == 0);

    TextBuffer_Deinit(&tb);
}

void test_textbuffer_text_around_gap(void) {
    TextBuffer tb;
    TextBuffer_Init(&tb);

    const char* text = "before-GAP-after";
    String_Set(&tb.current_line->text, String_FromCStr(text, strlen(text)));
    tb.gap.position = 10; // After "before-" and "GAP"
    tb.gap.overlap = 3;   // Overlaps "GAP"

    StringView before;
    StringView after;

    TextBuffer_TextAroundGap(&tb, &before, &after);

    String b = String_FromView(before);
    String a = String_FromView(after);
    check_string_equals(&b, "before-");
    check_string_equals(&a, "-after");

    String_Deinit(&a);
    String_Deinit(&b);

    TextBuffer_Deinit(&tb);
}

void test_textbuffer_line_management(void) {
    TextBuffer tb;
    TextBuffer_Init(&tb);
    Line *line1 = tb.current_line;
    Line *line0 = create_line("Line 0");
    Line *line2 = create_line("Line 2");
    Line *line3 = create_line("Line 3");
    
    TextBuffer_InsertLineAfterCurrent(&tb, line2);
    TEST_CHECK(tb.line_count == 2);
    TEST_CHECK(line1->next == line2);
    TEST_CHECK(line2->prev == line1);

    TextBuffer_InsertLineAtBottom(&tb, line3);
    TEST_CHECK(tb.line_count == 3);
    TEST_CHECK(line2->next == line3);
    TEST_CHECK(line3->prev == line2);
    TEST_CHECK(TextBuffer_GetLastLine(&tb) == line3);
    TEST_CHECK(TextBuffer_GetFirstLine(&tb) == line1);

    TextBuffer_InsertLineAtTop(&tb, line0);
    TEST_CHECK(tb.line_count == 4);
    TEST_CHECK(line1->prev == line0);
    TEST_CHECK(line0->next == line1);
    TEST_CHECK(TextBuffer_GetLastLine(&tb) == line3);
    TEST_CHECK(TextBuffer_GetFirstLine(&tb) == line0);

    // Cannot delete current line
    TEST_CHECK(TextBuffer_DeleteLine(&tb, line1) == false);
    TEST_CHECK(tb.line_count == 4);

    TEST_CHECK(TextBuffer_DeleteLine(&tb, line2) == true);
    TEST_CHECK(tb.line_count == 3);
    TEST_CHECK(line1->next == line3);
    TEST_CHECK(line3->prev == line1);

    TextBuffer_Deinit(&tb);
}

TEST_LIST = {
    { "TextBuffer: Init and Deinit", test_textbuffer_init_deinit },
    { "TextBuffer: MergeGap (Insert)", test_textbuffer_merge_gap_insert },
    { "TextBuffer: MergeGap (Overwrite)", test_textbuffer_merge_gap_overwrite },
    { "TextBuffer: TextAroundGap", test_textbuffer_text_around_gap },
    { "TextBuffer: Line Management", test_textbuffer_line_management },
    { NULL, NULL }
};