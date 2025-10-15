#include "acutest.h"
#include "common/typedtable.h"

void test_set_get(void) {
    Table *table = Table_Create();
    
    TypedTable_SetBoolean(table, "bool", true);
    TEST_CHECK(TypedTable_GetBoolean(table, "bool"));

    TypedTable_SetNumber(table, "number", 42);
    TEST_CHECK(TypedTable_GetNumber(table, "number") == 42);

    char *str = strdup("Foobar");
    TypedTable_SetString(table, "string", str);
    TEST_CHECK(TypedTable_GetString(table, "string") == str);

    TypedTable_SetStringCopy(table, "copy", "Blub");
    TEST_CHECK(strcmp(TypedTable_GetString(table, "copy"), "Blub") == 0);

    Table *subtable = Table_Create();
    TypedTable_SetTable(table, "subtable", subtable);
    TEST_CHECK(TypedTable_GetTable(table, "subtable") == subtable);

    TEST_CHECK(TypedTable_GetType(table, "bool") == VALUE_TYPE_BOOLEAN);
    TEST_CHECK(TypedTable_GetType(table, "number") == VALUE_TYPE_NUMBER);
    TEST_CHECK(TypedTable_GetType(table, "string") == VALUE_TYPE_STRING);
    TEST_CHECK(TypedTable_GetType(table, "copy") == VALUE_TYPE_STRING);
    TEST_CHECK(TypedTable_GetType(table, "subtable") == VALUE_TYPE_TABLE);
    TEST_CHECK(TypedTable_GetType(table, "invalid") == VALUE_TYPE_NONE);

    Table_Destroy(table);
}

TEST_LIST = {
    { "TypedTable: Set and Get", test_set_get },
    { NULL, NULL }
};
