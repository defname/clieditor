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
    
}

TEST_LIST = {
    { "TableIterator: Standard", test_standard },
    { NULL, NULL }
};

