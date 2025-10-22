#include "acutest.h"
#include "common/tableiterator.h"
#include "common/table.h"

void fill_table(const char **key, const char **values, size_t count) {
    Table *table = Table_Create();

    for (size_t i=0; i<count; i++) {
        Table_Set(table, key[i], (char*)values[i], NULL);
    }

    TableIterator it = TableIterator_Begin(table);
    size_t slots = 0;
    while (TableIterator_Next(&it)) {
        TEST_CHECK(it.current->state == TABLE_SLOT_USED);
        slots++;
    }
    TEST_CHECK(slots == count);
    Table_Destroy(table);
}

void test_standard(void) {
    const char *keys[] = {
        "key0",
        "key1",
        "key2",
        "key3"
    };

    const char *values[] = {
        "value0",
        "value1",
        "value2",
        "value3"
    };
    size_t count = sizeof(keys) / sizeof(keys[0]);

    fill_table(keys, values, count);
    fill_table(keys, values, 0);
    fill_table(keys, values, 1);
}

void test_iterating_twice(void) {
    Table *table = Table_Create();
    Table_Set(table, "key", "value", NULL);
    Table_Set(table, "key2", "value2", NULL);
    Table_Set(table, "key3", "value3", NULL);

    TableIterator it = TableIterator_Begin(table);
    size_t count = 0;
    while (TableIterator_Next(&it)) {
        count++;
    }
    TEST_CHECK(count == 3);

    // run second time without reinitializing
    count = 0;
    while (TableIterator_Next(&it)) {
        count++;
    }
    TEST_CHECK(count == 0);

    // run again with reinitializing
    it = TableIterator_Begin(table);
    count = 0;
    while (TableIterator_Next(&it)) {
        count++;
    }
    TEST_CHECK(count == 3);

    Table_Destroy(table);
}

void test_edgecases(void) {
    TableIterator it = TableIterator_Begin(NULL);
    TEST_CHECK(!TableIterator_Next(&it));

    TEST_CHECK(!TableIterator_Next(NULL));
}


TEST_LIST = {
    { "TableIterator: Standard", test_standard },
    { "TableIterator: Iterating twice", test_iterating_twice },
    { "TableIterator: Edgecases", test_edgecases },
    { NULL, NULL }
};

