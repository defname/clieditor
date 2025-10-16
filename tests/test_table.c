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

void test_set_get(void) {
    Table *table = Table_Create();

    // set with destructor
    Table_Set(table, "key0", strdup("Foobar"), free);

    // set without destructor
    char *s = strdup("Test");
    Table_Set(table, "key1", s, NULL);

    TEST_CHECK(table->used == 2);
    TEST_CHECK(table->capacity >= TABLE_INITIAL_CAPACITY);

    // get keys
    TEST_CHECK(strcmp(Table_Get(table, "key0"), "Foobar") == 0);
    TEST_CHECK(Table_Get(table, "key1") == s);
    // get non existing key
    TEST_CHECK(Table_Get(table, "invalid") == NULL);

    // overwrite
    Table_Set(table, "key0", strdup("Blub"), free);
    TEST_CHECK(strcmp(Table_Get(table, "key0"), "Blub") == 0);

    // delete
    Table_Delete(table, "key0");
    TEST_CHECK(Table_Get(table, "key0") == NULL);

    Table_Destroy(table);
    free(s);
}

void test_has_hasownership(void) {
    Table *table = Table_Create();

    char *s = strdup("Test");
    Table_Set(table, "key0", strdup("Foobar"), free);
    Table_Set(table, "key1", strdup("Blub"), free);
    Table_Set(table, "key2", s, NULL);
    Table_Set(table, "key3", NULL, NULL);

    TEST_CHECK(Table_Has(table, "key0"));
    TEST_CHECK(Table_Has(table, "key1"));
    TEST_CHECK(Table_Has(table, "key2"));
    TEST_CHECK(Table_Has(table, "key3"));
    TEST_CHECK(!Table_Has(table, "key4"));

    TEST_CHECK(Table_HasOwnership(table, "key0"));
    TEST_CHECK(Table_HasOwnership(table, "key1"));
    TEST_CHECK(!Table_HasOwnership(table, "key2"));
    TEST_CHECK(!Table_HasOwnership(table, "key3"));
    TEST_CHECK(!Table_HasOwnership(table, "key4"));

    Table_Destroy(table);
    free(s);
}

void test_rehashing(void) {
    Table *table = Table_Create();

    // increase table the first time
    Table_Set(table, "initial", NULL, NULL);
    TEST_CHECK(table->capacity == TABLE_INITIAL_CAPACITY);

    size_t orig_capacity = table->capacity;
    for (size_t i=1; i<orig_capacity; i++) {
        char a[30];
        snprintf(a, 30, "key%ld", i);
        Table_Set(table, a, strdup(a), free);
    }

    TEST_CHECK(table->capacity == TABLE_INITIAL_CAPACITY * TABLE_GROWTH_FACTOR);
    
    for (size_t i=1; i<orig_capacity; i++) {
        char a[30];
        snprintf(a, 30, "key%ld", i);
        TEST_CHECK(strcmp(Table_Get(table, a), a) == 0);
    }

    Table_Destroy(table);
}

void test_edge_case(void) {
    // fill the table up to max load
    // call get function. find_slot() will throw an error because
    // the table is too loaded
    Table *table = Table_Create();

    for (size_t i=0; i<TABLE_INITIAL_CAPACITY * TABLE_MAX_LOAD_FACTOR; i++) {
        char a[30];
        snprintf(a, 30, "key%ld", i);
        Table_Set(table, a, strdup(a), free);
    }

    Table_Get(table, "key0");
    
    Table_Destroy(table);
}

TEST_LIST = {
    { "Table: Creation", test_creation },
    { "Table: Set and Get", test_set_get },
    { "Table: Existence and Owndership", test_has_hasownership },
    { "Table: Rehashing", test_rehashing },
    { "Table: Edge Case", test_edge_case },
    { NULL, NULL }
};