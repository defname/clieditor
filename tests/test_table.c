#include "acutest.h"
#include "common/table.h"


void test_creation(void) {
    Table *table = Table_Create();
    TEST_ASSERT(table != NULL);
    TEST_CHECK(table->capacity == 0);
    TEST_CHECK(table->used == 0);
    TEST_CHECK(table->slots == NULL);
    Table_Destroy(table);
}

void test_table_access(void) {
    Table *table = Table_Create();

    // set int
    Table_Set(table, "key0", TABLE_VALUE_TYPE_INT, (TableValue)5);

    // set string
    char *ownership_will_be_taken = strdup("ownership will be taken");
    Table_Set(table, "key1", TABLE_VALUE_TYPE_STRING, (TableValue){ .string_value = ownership_will_be_taken });
    
    TableValue value;
    // get keys
    TEST_CHECK(Table_Get(table, "key0", &value, (TableValue)0) == TABLE_VALUE_TYPE_INT);
    TEST_CHECK(value.int_value == 5);
    TEST_CHECK(Table_Get(table, "key1", &value, (TableValue)0) == TABLE_VALUE_TYPE_STRING);
    TEST_CHECK(strcmp(value.string_value, "ownership will be taken") == 0);

    // overwrite
    Table_SetStr(table, "key0", "Foobar");
    TEST_ASSERT(Table_Get(table, "key0", &value, (TableValue)0) == TABLE_VALUE_TYPE_STRING);
    TEST_ASSERT(strcmp(value.string_value, "Foobar") == 0);

    // get non-existing key
    TEST_CHECK(Table_Get(table, "invalid", &value, (TableValue)0) == TABLE_VALUE_TYPE_NONE);

    // delete
    Table_Delete(table, "key0");
    TEST_CHECK(!Table_Has(table, "key0"));
    TEST_CHECK(Table_Get(table, "key0", &value, (TableValue)0) == TABLE_VALUE_TYPE_NONE);

    // test has
    TEST_CHECK(Table_Has(table, "key1"));
    TEST_CHECK(!Table_Has(table, "invalid"));

    // fallback
    TEST_CHECK(Table_Get(table, "invalid", &value, (TableValue){ .int_value = 42 }) == TABLE_VALUE_TYPE_NONE);
    TEST_CHECK(value.int_value == 42);

    Table_Destroy(table);
}

void test_rehashing(void) {
    Table *table = Table_Create();

    Table_SetInt(table, "int", 73);
    size_t orig_capacity = table->capacity;
    for (size_t i=1; i<orig_capacity; i++) {
        char a[30];
        snprintf(a, 30, "key%ld", i);
        Table_SetStr(table, a, a);
    }

    TEST_CHECK(table->capacity == TABLE_INITIAL_CAPACITY * TABLE_GROWTH_FACTOR);
    
    TableValue value;
    TEST_CHECK(Table_Get(table, "int", &value, (TableValue)0) == TABLE_VALUE_TYPE_INT);
    TEST_CHECK(value.int_value == 73);

    for (size_t i=1; i<orig_capacity; i++) {
        char a[30];
        snprintf(a, 30, "key%ld", i);
        TableValue value;
        TEST_CHECK(Table_Get(table, a, &value, (TableValue)0) == TABLE_VALUE_TYPE_STRING);
        TEST_CHECK(strcmp(value.string_value, a) == 0);
    }

    Table_Destroy(table);
}

TEST_LIST = {
    { "Table: Creation", test_creation },
    { "Table: Set and Get", test_table_access },
    { "Table: Rehashing", test_rehashing },
    { NULL, NULL }
};