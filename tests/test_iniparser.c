#include "acutest.h"
#include "common/iniparser.h"
#include "common/typedtable.h"

void test_simple(void) {
    IniParser parser;
    IniParser_Init(&parser);

    // basic key/value pairs
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

void test_spaces(void) {
    IniParser parser;
    IniParser_Init(&parser);

    // basic key/value pairs
    const char *text =
    "  barestring  =  foo bar \n"
    " string = \"foobar  \"  \n"
    "   number  =  42 \n";
    parser.text = text;
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table != NULL);
    TEST_CHECK(TypedTable_GetString(table, "barestring") != NULL);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "barestring"), "foo bar") == 0);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "string"), "foobar  ") == 0);
    TEST_CHECK(TypedTable_GetNumber(table, "number") == 42);

    Table_Destroy(table);
    IniParser_Deinit(&parser);
}

void test_empty_value(void) {
    IniParser parser;
    IniParser_Init(&parser);

    // basic key/value pairs
    const char *text =
    "barestring=  \n"
    "string=; comment\n"
    "empty=";
    parser.text = text;
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table != NULL);
    TEST_CHECK(TypedTable_GetString(table, "barestring") != NULL);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "barestring"), "") == 0);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "string"), "") == 0);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "empty"), "") == 0);
    
    Table_Destroy(table);
    IniParser_Deinit(&parser);
}

void test_value_redefining(void) {
    IniParser parser;
    IniParser_Init(&parser);

    // basic key/value pairs
    const char *text =
    "barestring=  \n"
    "barestring=foobar";
    parser.text = text;
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table != NULL);
    TEST_CHECK(TypedTable_GetString(table, "barestring") != NULL);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "barestring"), "foobar") == 0);
    
    Table_Destroy(table);
    IniParser_Deinit(&parser);
}

void test_escape_sequences(void) {
     IniParser parser;
    IniParser_Init(&parser);

    // basic key/value pairs
    const char *text =
    "escape=\"\\t\\n\\\"\\\\\"";
    parser.text = text;
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table != NULL);
    TEST_CHECK(TypedTable_GetString(table, "escape") != NULL);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "escape"), "\t\n\"\\") == 0);
    
    Table_Destroy(table);
    IniParser_Deinit(&parser);
}

void test_comments(void) {
    IniParser parser;
    IniParser_Init(&parser);

    const char *text =
    "barestring=foo bar # comment with multiple words\n"
    "# comment  with tab\n"
    ";comment\n"
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

void test_sections(void) {
    IniParser parser;
    IniParser_Init(&parser);

    // basic key/value pairs
    const char *text =
    "outside=outside\n"
    "[section1]\n"
    "value1= 123 \n"
    "\n"
    "[section2]\n"
    "[section3]\n"
    "number=42";
    parser.text = text;
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table != NULL);
    TEST_CHECK(TypedTable_GetString(table, "outside") != NULL);
    TEST_CHECK(strcmp(TypedTable_GetString(table, "outside"), "outside") == 0);
    Table *sec1 = TypedTable_GetTable(table, "section1");
    Table *sec2 = TypedTable_GetTable(table, "section2");
    Table *sec3 = TypedTable_GetTable(table, "section3");
    TEST_CHECK(sec1 != NULL);
    TEST_CHECK(sec2 != NULL);
    TEST_CHECK(sec3 != NULL);

    TEST_CHECK(TypedTable_GetNumber(sec1, "value1") == 123);
    TEST_CHECK(TypedTable_GetNumber(sec3, "number") == 42);

    Table_Destroy(table);
    IniParser_Deinit(&parser);
}

void test_error_position1(void) {
    const char *text1 =
    "barestring=foo bar \" ;test";
    IniParser parser;
    IniParser_Init(&parser);
    IniParser_SetText(&parser, text1);
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table == NULL);
    TEST_CHECK(IniParser_GetError(&parser) != NULL);
    TEST_CHECK(IniParser_GetError(&parser)->line == 1);
    TEST_CHECK(IniParser_GetError(&parser)->column == 19);
    IniParser_Deinit(&parser);
}

void test_error_position2(void) {
    const char *text1 =
    "barestring=foo bar ;test\n_23?";
    IniParser parser;
    IniParser_Init(&parser);
    IniParser_SetText(&parser, text1);
    Table *table = IniParser_Parse(&parser);
    TEST_CHECK(table == NULL);
    TEST_CHECK(IniParser_GetError(&parser) != NULL);
    TEST_CHECK(IniParser_GetError(&parser)->line == 2);
    TEST_CHECK(IniParser_GetError(&parser)->column == 3);
    IniParser_Deinit(&parser);
}

int check_success(const char *text) {
    IniParser parser;
    IniParser_Init(&parser);
    IniParser_SetText(&parser, text);
    Table *table = IniParser_Parse(&parser);
    int result = (table != NULL);
    IniParser_Deinit(&parser);
    if (table) {
        Table_Destroy(table);
    }
    return result;
}

void test_misc(void) {
    const char *valid_tests[] = {
        "[sec-tion]",
        "[sec.tion]",
        "[-section]",
        "-name=",
        "_name=",
        "name-=*fo_-?"
    };
    int valid_tests_count = sizeof(valid_tests)/sizeof(char*);
    const char *invalid_tests[] = {
        "number=123.3",
        "number=123d",
        "string=\"foo\\bar\"",
        "string=\"foobar\n",
        "[section",
        "[sec/tion]"
    };
    int invalid_tests_count = sizeof(invalid_tests)/sizeof(char*);

    for (int i=0; i<valid_tests_count; i++) {
        TEST_CHECK(valid_tests[i]);
    }

    for (int i=0; i<invalid_tests_count; i++) {
        TEST_CHECK(invalid_tests[i]);
    }
}

TEST_LIST = {
    { "IniParser: Parse", test_simple },
    { "IniParser: Spaces", test_spaces },
    { "IniParser: Empty Value", test_empty_value },
    { "IniParser: Redefine Value", test_value_redefining },
    { "IniParser: Escape Sequences", test_escape_sequences },
    { "IniParser: Comments", test_comments },
    { "IniParser: Sections", test_sections },
    { "IniParser: Error 1", test_error_position1 },
    { "IniParser: Error 2", test_error_position2 },
    { "IniParser: Misc", test_misc },
    { NULL, NULL }
};
