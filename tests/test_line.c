#include "acutest.h"

#include "document/line.h"

void test_create(void) {
    Line *line = Line_Create();
    TEST_CHECK(line->next == NULL);
    TEST_CHECK(line->prev == NULL);
    TEST_CHECK(line->position == 0);
    TEST_CHECK(String_Length(&line->text) == 0);
    Line_Destroy(line);
}

void test_insert_before(void) {
    Line *line1 = Line_Create();
    Line *line2 = Line_Create();
    Line *line3 = Line_Create();

    TEST_ASSERT(line1->position == 0);
    TEST_ASSERT(line2->position == 0);
    TEST_ASSERT(line3->position == 0);

    // line1, line3
    Line_InsertBefore(line3, line1);
    TEST_CHECK(line3->position == 0);
    TEST_CHECK(line1->position == -LINE_POSITION_STEP);
    TEST_CHECK(line1->next == line3);
    TEST_CHECK(line1->prev == NULL);
    TEST_CHECK(line3->prev == line1);
    TEST_CHECK(line3->next == NULL);
    
    // line1, line2, line3
    Line_InsertBefore(line3, line2);
    TEST_CHECK(line2->position == -LINE_POSITION_STEP/2);
    TEST_CHECK(line2->prev == line1);
    TEST_CHECK(line2->next == line3);

    Line_Destroy(line1);
    Line_Destroy(line2);
    Line_Destroy(line3);
}

void test_insert_after(void) {
    Line *line1 = Line_Create();
    Line *line2 = Line_Create();
    Line *line3 = Line_Create();

    TEST_ASSERT(line1->position == 0);
    TEST_ASSERT(line2->position == 0);
    TEST_ASSERT(line3->position == 0);

    // line1, line3
    Line_InsertAfter(line1, line3);
    TEST_CHECK(line1->position == 0);
    TEST_CHECK(line3->position == LINE_POSITION_STEP);
    TEST_CHECK(line1->next == line3);
    TEST_CHECK(line1->prev == NULL);
    TEST_CHECK(line3->prev == line1);
    TEST_CHECK(line3->next == NULL);
    
    // line1, line2, line3
    Line_InsertAfter(line1, line2);
    TEST_CHECK(line2->position == LINE_POSITION_STEP/2);
    TEST_CHECK(line2->prev == line1);
    TEST_CHECK(line2->next == line3);

    Line_Destroy(line1);
    Line_Destroy(line2);
    Line_Destroy(line3);
}

void test_rebuild_positions(void) {
    Line *line1 = Line_Create();
    Line *line2 = Line_Create();
    Line *line3 = Line_Create();
    
    Line_InsertAfter(line1, line3);
    line1->position = 1;
    line3->position = 2;

    Line_InsertAfter(line1, line2);
    TEST_CHECK(line1->position == 0);
    TEST_CHECK(line2->position == LINE_POSITION_STEP);
    TEST_CHECK(line3->position == 2 * LINE_POSITION_STEP);

    Line_Destroy(line1);
    Line_Destroy(line2);
    Line_Destroy(line3);
}

void test_delete(void) {
    Line *line1 = Line_Create();
    Line *line2 = Line_Create();
    Line *line3 = Line_Create();
    
    Line_InsertAfter(line1, line2);
    Line_InsertAfter(line2, line3);
    Line_Delete(line2);
    TEST_CHECK(line1->next == line3);
    TEST_CHECK(line3->prev == line1);
    Line_Destroy(line1);
    Line_Destroy(line3);
}

TEST_LIST = {
    { "Line: Create", test_create },
    { "Line: Insert before", test_insert_before },
    { "Line: Insert after", test_insert_after },
    { "Line: Rebuild positions", test_rebuild_positions },
    { "Line: Delete", test_delete },
    { NULL, NULL }
};
