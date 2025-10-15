#include "acutest.h"
#include "common/iniparser.h"
#include "common/typedtable.h"

void test_simple(void) {
    IniParser parser;
    IniParser_Init(&parser);
    const char *text =
    "barestring=foo bar \n"
    "string=\"foobar\" \n"
    "number=42 \n";
    parser.text = text;
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table != NULL);
    TEST_CHECK(TypedTable_GetString(table, "barestring") != NULL);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "barestring"), "foo bar") == 0);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "string"), "foobar") == 0);
    TEST_CHECK(TypedTable_GetNumber(table, "number") == 42);

    
    Table_Destroy(table);
    IniParser_Deinit(&parser);
}

TEST_LIST = {
    { "IniParser: Parse", test_simple },
    { NULL, NULL }
};
