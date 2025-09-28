#include "acutest.h"

// Inkludiere die C-Datei direkt.
// widget.c inkludiert widget.h, logging.h und stdlib.h
#include "core/widget.c"

void test_widget_creation(void) {
    Widget *w = Widget_Create(NULL, NULL);
    TEST_ASSERT(w != NULL);
    TEST_CHECK(w->parent == NULL);
    TEST_CHECK(w->children == NULL);
    TEST_CHECK(w->children_count == 0);
    TEST_CHECK(w->children_capacity == 0);

    Widget_Destroy(w);
}

void test_widget_hierarchy(void) {
    Widget *parent = Widget_Create(NULL, NULL);
    Widget *child1 = Widget_Create(NULL, NULL);
    Widget *child2 = Widget_Create(NULL, NULL);

    TEST_ASSERT(parent != NULL && child1 != NULL && child2 != NULL);

    TEST_CASE("Add children");
    Widget_AddChild(parent, child1);
    TEST_CHECK(parent->children_count == 1);
    TEST_CHECK(parent->children[0] == child1);
    TEST_CHECK(child1->parent == parent);

    Widget_AddChild(parent, child2);
    TEST_CHECK(parent->children_count == 2);
    TEST_CHECK(parent->children[1] == child2);
    TEST_CHECK(child2->parent == parent);

    TEST_CASE("Remove child");
    Widget_RemoveChild(parent, child1);
    TEST_CHECK(parent->children_count == 1);
    TEST_CHECK(child1->parent == NULL);
    // Check that child2 is still there
    TEST_CHECK(parent->children[0] == child2);
    Widget_Destroy(parent);
    Widget_Destroy(child1);
    Widget_Destroy(child2);
}


TEST_LIST = {
    { "widget_creation", test_widget_creation },
    { "widget_hierarchy", test_widget_hierarchy },
    { NULL, NULL }
};