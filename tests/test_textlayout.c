#include <locale.h>
#include <string.h>

// Initialize locale for every test to ensure wcwidth and other functions work correctly with UTF-8
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "document/textbuffer.h"
#include "document/textlayout.h"
#include "document/line.h"

// --- Test Setup Helper Functions ---

typedef struct {
    TextBuffer tb;
    TextLayout tl;
} TestFixture;

// Initializes a test fixture with an empty buffer and layout
static void setup_fixture(TestFixture* f, int width, int height) {
    TextBuffer_Init(&f->tb);
    TextLayout_Init(&f->tl, &f->tb, width, height);
}

// Adds lines to the TextBuffer
static void add_lines(TextBuffer* tb, const char* lines[], int count) {
    // Replace the first, empty line
    String_Set(&tb->current_line->text, String_FromCStr(lines[0], strlen(lines[0])));

    for (int i = 1; i < count; i++) {
        Line* new_line = Line_Create();
        String_Set(&new_line->text, String_FromCStr(lines[i], strlen(lines[i])));
        TextBuffer_InsertLineAtBottom(tb, new_line);
    }
}

// Frees the resources of the fixture
static void teardown_fixture(TestFixture* f) {
    TextLayout_Deinit(&f->tl);
    TextBuffer_Deinit(&f->tb);
}

// --- Tests ---

void test_layout_init_and_dimensions(void) {
    TestFixture f;
    setup_fixture(&f, 80, 25);

    TEST_CHECK(f.tl.width == 80);
    TEST_CHECK(f.tl.height == 25);
    TEST_CHECK(f.tl.dirty == true);
    TEST_CHECK(f.tl.first_line == f.tb.current_line);

    TextLayout_SetDimensions(&f.tl, 100, 30);
    TEST_CHECK(f.tl.width == 100);
    TEST_CHECK(f.tl.height == 30);
    TEST_CHECK(f.tl.dirty == true);

    teardown_fixture(&f);
}

void test_recalc_simple_no_wrap(void) {
    TestFixture f;
    setup_fixture(&f, 80, 25);
    const char* lines[] = {"line 1", "line 2", "line 3"};
    add_lines(&f.tb, lines, 3);

    TextLayout_Recalc(&f.tl, 0);

    VisualLine* vl0 = TextLayout_GetVisualLine(&f.tl, 0);
    TEST_ASSERT(vl0 != NULL);
    TEST_CHECK(vl0->offset == 0);
    TEST_CHECK(vl0->length == 6);
    TEST_CHECK(vl0->width == 6);
    TEST_CHECK(vl0->src == f.tb.current_line);

    VisualLine* vl1 = TextLayout_GetVisualLine(&f.tl, 1);
    TEST_ASSERT(vl1 != NULL);
    TEST_CHECK(vl1->offset == 0);
    TEST_CHECK(vl1->length == 6);
    TEST_CHECK(vl1->width == 6);
    TEST_CHECK(vl1->src == f.tb.current_line->next);

    // Line 3 should also be visible
    VisualLine* vl2 = TextLayout_GetVisualLine(&f.tl, 2);
    TEST_ASSERT(vl2 != NULL);

    // Outside of the document
    VisualLine* vl3 = TextLayout_GetVisualLine(&f.tl, 3);
    TEST_CHECK(vl3 == NULL);

    teardown_fixture(&f);
}

void test_recalc_line_wrapping(void) {
    TestFixture f;
    setup_fixture(&f, 10, 25); // Very narrow width to force wrapping
    const char* lines[] = {"This is a very long line that must wrap."};
    add_lines(&f.tb, lines, 1);

    TextLayout_Recalc(&f.tl, 0);

    // "This is a " (10 characters, 10 width)
    VisualLine* vl0 = TextLayout_GetVisualLine(&f.tl, 0);
    TEST_ASSERT(vl0 != NULL);
    TEST_CHECK(vl0->offset == 0);
    TEST_CHECK(vl0->length == 10);
    TEST_CHECK(vl0->width == 10);

    // "very long " (10 characters, 10 width)
    VisualLine* vl1 = TextLayout_GetVisualLine(&f.tl, 1);
    TEST_ASSERT(vl1 != NULL);
    TEST_CHECK(vl1->offset == 10);
    TEST_CHECK(vl1->length == 10);
    TEST_CHECK(vl1->width == 10);
    TEST_CHECK(vl1->src == vl0->src); // Must be the same source line

    // "line that " (10 characters, 10 width)
    VisualLine* vl2 = TextLayout_GetVisualLine(&f.tl, 2);
    TEST_ASSERT(vl2 != NULL);
    TEST_CHECK(vl2->offset == 20);
    TEST_CHECK(vl2->length == 10);

    // "must wrap." (10 characters, 10 width)
    VisualLine* vl3 = TextLayout_GetVisualLine(&f.tl, 3);
    TEST_ASSERT(vl3 != NULL);
    TEST_CHECK(vl3->offset == 30);
    TEST_CHECK(vl3->length == 10);

    // Next visual line should be empty
    VisualLine* vl4 = TextLayout_GetVisualLine(&f.tl, 4);
    TEST_CHECK(vl4 == NULL);

    teardown_fixture(&f);
}

void test_recalc_wide_chars_and_tabs(void) {
    TestFixture f;
    setup_fixture(&f, 20, 25);
    f.tl.tabstop = 4;
    // "Hi\t😊\t(10)" -> Width: 2 + 2 (Tab) + 2 (Emoji) + 1 (Tab) + 4 = 11
    const char* lines[] = {"Hi\t😊\t(10)"};
    add_lines(&f.tb, lines, 1);

    TextLayout_Recalc(&f.tl, 0);

    VisualLine* vl0 = TextLayout_GetVisualLine(&f.tl, 0);
    TEST_ASSERT(vl0 != NULL);
    TEST_CHECK(vl0->length == 9); // H,i,\t,😊,\t,(,1,0,) -> 9 characters
    TEST_CHECK(vl0->width == 12);
    TEST_MSG("Expected width 11, got %d", vl0->width);

    teardown_fixture(&f);
}

void test_cursor_position(void) {
    TestFixture f;
    setup_fixture(&f, 10, 5);
    const char* lines[] = {"0123456789", "abcdefghij", "klmnopqrst"};
    add_lines(&f.tb, lines, 3);
    f.tb.current_line = f.tb.current_line->next; // Cursor on line "abc..."

    // Cursor at the beginning of line 1 (visual line 1)
    CursorLayoutInfo info;
    f.tb.gap.position = 0;
    TextLayout_Recalc(&f.tl, 0);
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.on_screen == 0);
    TEST_CHECK(info.y == 1);
    TEST_CHECK(info.x == 0);
    TEST_CHECK(info.idx == 0);

    f.tb.gap.position = 5;
    TextLayout_Recalc(&f.tl, 0);
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.y == 1);
    TEST_CHECK(info.x == 5);
    TEST_CHECK(info.idx == 5);

    // Cursor in the middle of line 1 (visual line 1)
    f.tb.gap.position = 5;
    f.tl.dirty = true; // Force recalculation
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.on_screen == 0);
    TEST_CHECK(info.y == 1);
    TEST_CHECK(info.x == 5);
    TEST_CHECK(info.idx == 5);

    // Move cursor to the 3rd line, which is wrapped
    String_Set(&f.tb.current_line->next->text, String_FromCStr("This is a very long line.", 25));
    f.tb.current_line = f.tb.current_line->next;
    f.tb.gap.position = 15; // "very |l|ong line"
    f.tl.dirty = true;
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    // Visual lines: "0123456789", "abcdefghij", "This is a ", "very long ", "line."
    // Cursor should be on visual line 3 (index 3)
    TEST_CHECK(info.on_screen == 0);
    TEST_CHECK(info.y == 3);
    TEST_CHECK(info.x == 5);
    TEST_MSG("%d", info.x);
    TEST_CHECK(info.idx == 5);

    teardown_fixture(&f);
}

void test_scrolling(void) {
    TestFixture f;
    setup_fixture(&f, 10, 3); // Small window
    const char* lines[] = {"Line 0", "Line 1 wraps", "Line 2", "Line 3", "Line 4"};
    add_lines(&f.tb, lines, 5);

    TextLayout_Recalc(&f.tl, 0);

    // Initial state: "Line 0", "Line 1 wra", "ps"
    TEST_ASSERT(strcmp(String_AsCStr(&TextLayout_GetVisualLine(&f.tl, 0)->src->text), "Line 0") == 0);

    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->src->position == 0);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->offset == 0);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->length == 6);  // "Line 0"
        
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->src != TextLayout_GetVisualLine(&f.tl, 1)->src);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 1)->offset == 0);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 2)->offset == 10);

    // 0. Scroll Up should fail
    TEST_CHECK(TextLayout_ScrollUp(&f.tl) == false);

    // 1. Scroll Down: Should jump to the next line
    TEST_CHECK(TextLayout_ScrollDown(&f.tl) == true);
    // New state: "Line 1 wra", "ps", "Line 2"
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->src->position == 1*LINE_POSITION_STEP);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->offset == 0);

    // 2. Scroll Down: Should scroll within the wrapped line
    TEST_CHECK(TextLayout_ScrollDown(&f.tl) == true);
    // New state: "ps", "Line 2", "Line 3"
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->src->position == 1*LINE_POSITION_STEP);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->offset == 10);
    
    // 3. Scroll Up: Back to the beginning of the wrapped line
    TEST_CHECK(TextLayout_ScrollUp(&f.tl) == true);

    // Zustand: "Line 1 wra", "ps", "Line 2"
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->src->position == 1*LINE_POSITION_STEP);
    TEST_CHECK(TextLayout_GetVisualLine(&f.tl, 0)->offset == 0);

    teardown_fixture(&f);
}

void test_scrolling_down(void) {
    TestFixture f;
    setup_fixture(&f, 10, 3);
    //                     |1st line  |2nd line  |3rd line  |4th line
    //                     "Long line |wrapped in| four line|s."
    const char* lines[] = {"Long line wrapped in four lines.", "Short."};
    add_lines(&f.tb, lines, 2);

    // "Long line ", "wrapped in", " four line"
    TextLayout_Recalc(&f.tl, 0);
    TEST_CHECK(f.tl.cache_capacity == 6);
    TEST_CHECK(f.tl.cache[0].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[0].offset == 0);
    TEST_CHECK(f.tl.cache[0].length == 10);
    TEST_CHECK(f.tl.cache[1].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[1].offset == 10);
    TEST_CHECK(f.tl.cache[1].length == 10);
    TEST_CHECK(f.tl.cache[2].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[2].offset == 20);
    TEST_CHECK(f.tl.cache[2].length == 10);
    TEST_CHECK(f.tl.cache[3].src == NULL);

    // "Long line ", "wrapped in", " four line", "s."
    TEST_CHECK(TextLayout_ScrollDown(&f.tl));
    TEST_CHECK(f.tl.first_visual_line_idx == 1);
    TEST_CHECK(f.tl.cache_capacity == 6);
    TEST_CHECK(f.tl.cache[0].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[0].offset == 0);
    TEST_CHECK(f.tl.cache[0].length == 10);
    TEST_CHECK(f.tl.cache[1].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[1].offset == 10);
    TEST_CHECK(f.tl.cache[1].length == 10);
    TEST_CHECK(f.tl.cache[2].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[2].offset == 20);
    TEST_CHECK(f.tl.cache[2].length == 10);
    TEST_CHECK(f.tl.cache[3].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[3].offset == 30);
    TEST_CHECK(f.tl.cache[3].length == 2);
    TEST_CHECK(f.tl.cache[4].src == NULL);

    // "Long line ", "wrapped in", " four line", "s.", "Short."
    TEST_CHECK(TextLayout_ScrollDown(&f.tl));
    TEST_CHECK(f.tl.first_visual_line_idx == 2);
    TEST_CHECK(f.tl.cache_capacity == 6);
    TEST_CHECK(f.tl.cache[3].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[3].offset == 30);
    TEST_CHECK(f.tl.cache[3].length == 2);
    TEST_CHECK(f.tl.cache[4].src == f.tb.current_line->next);
    TEST_CHECK(f.tl.cache[4].offset == 0);
    TEST_CHECK(f.tl.cache[4].length == 6);
    TEST_MSG("Expected length 6, got %d", f.tl.cache[4].length);
    TEST_CHECK(f.tl.cache[5].src == NULL);

    TEST_CHECK(TextLayout_ScrollDown(&f.tl));
    TEST_CHECK(f.tl.first_visual_line_idx == 3);
    TEST_CHECK(f.tl.cache[5].src == NULL);

    TEST_CHECK(TextLayout_ScrollDown(&f.tl) == false);
    TEST_CHECK(f.tl.first_visual_line_idx == 3);

    teardown_fixture(&f);
}

void test_scrolling_up(void) {
    TestFixture f;
    setup_fixture(&f, 10, 3); 

    const char* lines[] = {"Short.", "Long line wrapped in four lines.", "Short."};
    add_lines(&f.tb, lines, 3);
    TextLayout_SetFirstLine(&f.tl, f.tb.current_line->next, 3);

    TextLayout_Recalc(&f.tl, -3);

    // "s.", "Short." on screen
    TEST_CHECK(f.tl.cache[0].src == f.tb.current_line->next);
    TEST_CHECK(f.tl.cache[0].offset == 0);
    TEST_CHECK(f.tl.cache[3].src == f.tb.current_line->next);
    TEST_CHECK(f.tl.cache[3].offset == 30);
    TEST_CHECK(f.tl.cache[3].length == 2);
    TEST_CHECK(f.tl.cache[4].length == 6);
    TEST_CHECK(f.tl.cache[5].src == NULL);

    // Scroll up
    TEST_CHECK(TextLayout_ScrollUp(&f.tl) == true);
    TEST_CHECK(f.tl.first_visual_line_idx == 2);
    TEST_CHECK(f.tl.cache[4].length == 6);  // this line stays in cache, cause no recalc was needed

    // Scroll up
    TEST_CHECK(TextLayout_ScrollUp(&f.tl) == true);
    TEST_CHECK(f.tl.first_visual_line_idx == 1);

    // Scroll up
    TEST_CHECK(TextLayout_ScrollUp(&f.tl) == true);
    TEST_CHECK(f.tl.first_visual_line_idx == 0);

    // Scroll up
    TEST_CHECK(TextLayout_ScrollUp(&f.tl) == true);
    TEST_CHECK(f.tl.first_visual_line_idx == 0);
    TEST_CHECK(f.tl.cache[0].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[0].offset == 0);
    TEST_CHECK(f.tl.cache[0].length == 6);

    TEST_CHECK(f.tl.cache[3].src == NULL);
    
    teardown_fixture(&f);
}

void test_cursor_position2(void) {
    TestFixture f;
    setup_fixture(&f, 10, 5);
    const char *lines[] = {
        "0\t45678901234",
        "\t\t89",
        "0123456789",
        "\t12345"
    };
    add_lines(&f.tb, lines, 4);
    f.tl.tabstop = 4;
    f.tl.first_visual_line_idx = 1;

    // VisualLines in cache
    // "0\t456789", "01234", "0123", "0123456789", "\t12345"

    // 0   1
    f.tb.gap.position = 8;
    TextLayout_Recalc(&f.tl, -1);
    TEST_CHECK(f.tl.cache[0].char_x[0] == 0); // "0"
    TEST_CHECK(f.tl.cache[0].char_x[1] == 1); // "\t"
    TEST_CHECK(f.tl.cache[0].char_x[2] == 4); // "4"


    CursorLayoutInfo info;
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.on_screen == 0);
    TEST_CHECK(info.y == 0);
    TEST_CHECK(info.x == 0);
    TEST_MSG("%d", info.x);
    TEST_CHECK(info.idx == 0);

    f.tb.gap.position = 1;
    TextLayout_Recalc(&f.tl, -1);
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.on_screen == -1);

    TEST_CHECK(TextLayout_ScrollUp(&f.tl));
    TEST_ASSERT(f.tl.first_visual_line_idx == 0);
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.on_screen == 0);
    TEST_CHECK(info.y == 0);
    TEST_CHECK(info.x == 1);
    TEST_CHECK(info.idx == 1);


    f.tb.gap.position = 2;
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.y == 0);
    TEST_CHECK(info.x == 4);
    TEST_CHECK(info.idx == 2);

    f.tb.current_line = f.tb.current_line->next; // "\t\t78"
    f.tb.gap.position = 4;  // end of line
    TextLayout_GetCursorLayoutInfo(&f.tl, &info);
    TEST_CHECK(info.y == 2);
    TEST_CHECK(info.x == 10);
    TEST_MSG("%d", info.x);
    TEST_CHECK(info.idx == 4);

    teardown_fixture(&f);
}

void test_gap_sensitivity(void) {
     TestFixture f;
    setup_fixture(&f, 10, 5);
    const char *lines[] = {
        "0\t2",
        "0123456789abcdef",
        "ABCDEF"
    };
    add_lines(&f.tb, lines, 3);

    TextLayout_Recalc(&f.tl, 0);
    TEST_CHECK(f.tl.cache[0].length == 3);

    f.tl.tabstop = 4;
    f.tb.gap.position = 0;
    f.tb.gap.overlap = 0;
    String_Set(&f.tb.gap.text, String_FromCStr("abc", 3));

    TextLayout_Recalc(&f.tl, 0);
    TEST_CHECK(f.tl.cache[0].gap != NULL);
    TEST_CHECK(f.tl.cache[0].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[0].length == 6);  // "a", "b", "c", "0", "\t", "2"
    TEST_CHECK(f.tl.cache[0].width == 9);

    TEST_CHECK(VisualLine_GetChar(&f.tl.cache[0], 0)[0] == 'a');
    TEST_MSG("%c", VisualLine_GetChar(&f.tl.cache[0], 0)[0]);
    TEST_CHECK(VisualLine_GetChar(&f.tl.cache[0], 3)[0] == '0');

    f.tb.gap.position = 2;
    f.tb.gap.overlap = 1;
    TextLayout_Recalc(&f.tl, 0);
    TEST_CHECK(f.tl.cache[0].gap != NULL);
    TEST_CHECK(f.tl.cache[0].length == 5);
    TEST_CHECK(VisualLine_GetChar(&f.tl.cache[0], 1)[0] == 'a');

    f.tb.current_line = f.tb.current_line->next;
    f.tb.gap.position = 3;
    f.tb.gap.overlap = 1;
    String_Set(&f.tb.gap.text, String_FromCStr("", 0));

    TextLayout_Recalc(&f.tl, 0);
    TEST_CHECK(f.tl.cache[0].gap == NULL);
    TEST_CHECK(f.tl.cache[1].gap != NULL);
    TEST_CHECK(f.tl.cache[2].gap != NULL);
    TEST_CHECK(f.tl.cache[3].gap == NULL);

    teardown_fixture(&f);
}

TEST_LIST = {
    { "TextLayout: Init and Dimensions", test_layout_init_and_dimensions },
    { "TextLayout: Recalc (Simple, no wrap)", test_recalc_simple_no_wrap },
    { "TextLayout: Recalc (Line Wrapping)", test_recalc_line_wrapping },
    { "TextLayout: Recalc (Wide Chars and Tabs)", test_recalc_wide_chars_and_tabs },
    { "TextLayout: Cursor Position", test_cursor_position },
    { "TextLayout: Scrolling (Up/Down)", test_scrolling },
    { "TextLayout: Scrolling Down Edge Cases", test_scrolling_down },
    { "TextLayout: Scrolling Up Edge Cases", test_scrolling_up },
    { "TextLayout: Cursor Position Advanced", test_cursor_position2 },
    { "TextLayout: Gap sensitivity", test_gap_sensitivity },
    { NULL, NULL }
};
