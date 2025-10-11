#include <locale.h>
#include <string.h>

// Initialize locale for every test to ensure wcwidth and other functions work correctly with UTF-8
#define TEST_INIT setlocale(LC_ALL, "");

#include "acutest.h"
#include "document/textbuffer.h"
#include "document/textlayout.h"
#include "document/textedit.h"
#include "document/line.h"

// --- Test Setup Helper Functions ---

typedef struct {
    TextBuffer tb;
    TextLayout tl;
    TextEdit te;
} TestFixture;

// Initializes a test fixture with buffer, layout, and editor
static void setup_fixture(TestFixture* f, int width, int height) {
    TextBuffer_Init(&f->tb);
    TextLayout_Init(&f->tl, &f->tb, width, height);
    TextEdit_Init(&f->te, &f->tb, &f->tl);
}

// Adds lines to the TextBuffer and sets the cursor to the beginning
static void add_lines(TextBuffer* tb, const char* lines[], int count) {
    if (count == 0) return;
    // Replace the first, empty line
    UTF8String_FromStr(&tb->current_line->text, lines[0], strlen(lines[0]));

    for (int i = 1; i < count; i++) {
        Line* new_line = Line_Create();
        UTF8String_FromStr(&new_line->text, lines[i], strlen(lines[i]));
        TextBuffer_InsertLineAfterCurrent(tb, new_line);
        tb->current_line = new_line;
    }
    // Reset to first line
    tb->current_line = TextBuffer_GetFirstLine(tb);
    tb->gap.position = 0;
}

// Frees the resources of the fixture
static void teardown_fixture(TestFixture* f) {
    TextEdit_Deinit(&f->te);
    TextLayout_Deinit(&f->tl);
    TextBuffer_Deinit(&f->tb);
}

// --- Tests ---

void test_move_left_right_simple(void) {
    TestFixture f;
    setup_fixture(&f, 80, 25);
    const char* lines[] = {"line 1", "line 2"};
    add_lines(&f.tb, lines, 2);

    // Start at line 1, pos 0
    f.tb.gap.position = 0;

    // Move right
    TextEdit_MoveRight(&f.te);
    TEST_CHECK(f.tb.gap.position == 1);
    TEST_CHECK(f.tb.current_line->position == 0); // Still on line 1

    // Move to end of line 1
    f.tb.gap.position = 6;
    TextEdit_MoveRight(&f.te); // Should wrap to line 2
    TEST_CHECK(f.tb.current_line->position == 1 * LINE_POSITION_STEP); // Now on line 2
    TEST_CHECK(f.tb.gap.position == 0);

    // Move right at end of document
    f.tb.current_line = TextBuffer_GetLastLine(&f.tb);
    f.tb.gap.position = 6;
    TextEdit_MoveRight(&f.te);
    TEST_CHECK(f.tb.current_line->position == 1 * LINE_POSITION_STEP); // Still on line 2
    TEST_CHECK(f.tb.gap.position == 6); // Stays at the end

    // Move left from start of line 2
    f.tb.current_line = TextBuffer_GetLastLine(&f.tb);
    f.tb.gap.position = 0;
    TextEdit_MoveLeft(&f.te); // Should wrap to line 1
    TEST_CHECK(f.tb.current_line->position == 0); // Now on line 1
    TEST_CHECK(f.tb.gap.position == 6); // At the end of line 1

    // Move left at start of document
    f.tb.current_line = TextBuffer_GetFirstLine(&f.tb);
    f.tb.gap.position = 0;
    TextEdit_MoveLeft(&f.te);
    TEST_CHECK(f.tb.current_line->position == 0); // Stays on line 1
    TEST_CHECK(f.tb.gap.position == 0); // Stays at the beginning

    teardown_fixture(&f);
}

void test_move_up_down_simple(void) {
    TestFixture f;
    setup_fixture(&f, 80, 25);
    const char* lines[] = {"long line one", "short", "long line three"};
    add_lines(&f.tb, lines, 3);

    // Start at line 3, pos 7 ("long li|ne three")
    f.tb.current_line = TextBuffer_GetLastLine(&f.tb);
    f.tb.gap.position = 7;
    TextLayout_Recalc(&f.tl, 0);
    
    TEST_ASSERT(f.tb.current_line->position == 2 * LINE_POSITION_STEP);
    TEST_ASSERT(f.tl.first_visual_line_idx == 0);
    //TEST_ASSERT(TextLayout_GetCursorY(&f.tl) == 2);
    //TEST_ASSERT(TextLayout_GetCursorX(&f.tl) == 7);

    // Move Up to "short"
    TextEdit_MoveUp(&f.te);
    TEST_CHECK(f.tb.current_line->position == 1 * LINE_POSITION_STEP); // On line "short"
    TEST_CHECK(f.tb.gap.position == 5); // Clamped to end of "short"
    TEST_CHECK(f.tl.cache[1].src == f.tb.current_line);
    TEST_CHECK(f.tl.cache[1].offset == 0);
    TEST_CHECK(f.tl.cache[1].length == 5);
    TEST_CHECK(f.tb.current_line->prev != NULL);
    TEST_CHECK(f.tb.current_line->prev->position == 0);
    //TEST_CHECK(TextLayout_GetCursorY(&f.tl) == 1);
    //TEST_MSG("Expected cursor y == 1, but got %d", TextLayout_GetCursorY(&f.tl));


    // Move Up to "long line one"
    TextEdit_MoveUp(&f.te);
    TEST_CHECK(f.tb.current_line->position == 0); // On line "long line one"
    TEST_CHECK(f.tb.gap.position == 5); // x-pos is preserved

    // Move Up at top of document
    TextEdit_MoveUp(&f.te);
    TEST_CHECK(f.tb.current_line->position == 0); // Stays on line 1
    TEST_CHECK(f.tb.gap.position == 0); // Jumps to beginning of line

    // Move Down to "short"
    TextEdit_MoveDown(&f.te);
    TEST_CHECK(f.tb.current_line->position == 1 * LINE_POSITION_STEP); // On line "short"
    TEST_CHECK(f.tb.gap.position == 0); // x-pos was 0

    // Move Down to "long line three"
    TextEdit_MoveDown(&f.te);
    TEST_CHECK(f.tb.current_line->position == 2 * LINE_POSITION_STEP); // On line 3
    TEST_CHECK(f.tb.gap.position == 0);

    // Move Down at end of document
    TextEdit_MoveDown(&f.te);
    TEST_CHECK(f.tb.current_line->position == 2 * LINE_POSITION_STEP); // Stays on line 3
    TEST_CHECK(f.tb.gap.position == 15); // Jumps to end of line

    teardown_fixture(&f);
}

void test_move_up_down_with_wrapping(void) {
    TestFixture f;
    setup_fixture(&f, 10, 5); // Narrow view to force wrapping
    const char* lines[] = {"0\t456789ABCDEFGHIJ"};
    add_lines(&f.tb, lines, 1);

    // Start at pos 15 ("...CDE|FGHIJ")
    f.tb.gap.position = 12;
    TextLayout_Recalc(&f.tl, 0);
    // Visual lines: "0123456789", "ABCDEFGHIJ"
    // Cursor is on visual line 1, x-pos 5
    //TEST_CHECK(TextLayout_GetCursorY(&f.tl) == 1);

    // Move Up
    TextEdit_MoveUp(&f.te);
    TEST_CHECK(f.tb.current_line->position == 0); // Still on same logical line
    TEST_CHECK(f.tb.gap.position == 2); // Moves to x-pos 5 on visual line 0

    // Move Down
    TextEdit_MoveDown(&f.te);
    TEST_CHECK(f.tb.gap.position == 12); // Back to original position

    teardown_fixture(&f);
}

TEST_LIST = {
    { "TextEdit: Move Left/Right (Simple)", test_move_left_right_simple },
    { "TextEdit: Move Up/Down (Simple & x-pos clamping)", test_move_up_down_simple },
    { "TextEdit: Move Up/Down (with Line Wrapping)", test_move_up_down_with_wrapping },
    { NULL, NULL }
};