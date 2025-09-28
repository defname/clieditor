#include "acutest.h"
#include "../src/widgets/label.h"

// Inkludiere die C-Dateien, die wir für den Test brauchen
#include "../src/widgets/label.c"

void test_label_creation_and_destruction(void) {
    Widget* label = Label_Create(NULL, "Hallo Welt");

    TEST_ASSERT(label != NULL);
    TEST_ASSERT(label->ops == &label_ops); // Prüft, ob es ein Label ist
    TEST_ASSERT(label->data != NULL);

    //LabelData* data = (LabelData*)label->data;
    //TEST_CHECK(strcmp(data->text, "Hallo Welt") == 0);

    // Ruft label_destroy auf, was wiederum Widget_Destroy aufruft
    label->ops->destroy(label);
}

void test_label_draw(void) {
    Widget* label = Label_Create(NULL, "Test");
    Canvas canvas;
    Canvas_Init(&canvas, 10, 1);

    label->ops->draw(label, &canvas);

    // Hier könnte man den Inhalt des Canvas-Buffers prüfen
    TEST_CHECK(UTF8_EqualToChar(canvas.buffer[0].ch, 'T'));
    TEST_CHECK(UTF8_EqualToChar(canvas.buffer[1].ch, 'e'));

    Canvas_Deinit(&canvas);
    label->ops->destroy(label);
}

TEST_LIST = {
    { "label_creation_and_destruction", test_label_creation_and_destruction },
    { "label_draw", test_label_draw },
    { NULL, NULL }
};