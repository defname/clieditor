#include "acutest.h"
#include "syntax/definition.h"
#include "common/iniparser.h"



void test_block_simple(void) {
    Table *block_table = Table_Create();
    TypedTable_SetStringCopy(block_table, "start", ".*");
    TypedTable_SetNumber(block_table, "color", 10);
    SyntaxDefinitionError error;
    SyntaxBlockDef *block = SyntaxBlockDef_FromTable("test", block_table, &error);
    TEST_CHECK(block != NULL);
    TEST_CHECK(block->name != NULL);
    TEST_CHECK(strcmp(block->name, "test") == 0);
    TEST_CHECK(block->start.re_nsub == 0);
    TEST_CHECK(block->color == 10);
    SyntaxBlockDef_Destroy(block);
    Table_Destroy(block_table);
    SyntaxDefinitionError_Deinit(&error);
}

void test_block_with_end(void) {
    Table *block_table = Table_Create();
    TypedTable_SetStringCopy(block_table, "start", ".*");
    TypedTable_SetStringCopy(block_table, "end", ".*");
    TypedTable_SetNumber(block_table, "color", 10);
    SyntaxDefinitionError error;
    SyntaxBlockDef *block = SyntaxBlockDef_FromTable("test", block_table, &error);
    TEST_CHECK(block != NULL);
    TEST_CHECK(block->name != NULL);
    TEST_CHECK(strcmp(block->name, "test") == 0);
    TEST_CHECK(block->start.re_nsub == 0);
    TEST_CHECK(block->color == 10);
    SyntaxBlockDef_Destroy(block);
    Table_Destroy(block_table);
    SyntaxDefinitionError_Deinit(&error);
}

void test_block_errors(void) {
    Table *block_table = Table_Create();

    // no start regex
    SyntaxDefinitionError error;
    SyntaxBlockDef *block = SyntaxBlockDef_FromTable("test", block_table, &error);
    TEST_CHECK(block == NULL);
    TEST_CHECK(error.code == SYNTAXDEFINITION_BLOCK_NO_START_REGEX);
    SyntaxDefinitionError_Deinit(&error);

    // error in start regex
    TypedTable_SetStringCopy(block_table, "start", "(\\(");
    block = SyntaxBlockDef_FromTable("test", block_table, &error);
    TEST_CHECK(block == NULL);
    TEST_CHECK(error.code == SYNTAXDEFINITION_REGEX_ERROR_START);
    SyntaxDefinitionError_Deinit(&error);

    // error in end regex
    TypedTable_SetStringCopy(block_table, "start", ".*");
    TypedTable_SetStringCopy(block_table, "end", "(\\(");
    block = SyntaxBlockDef_FromTable("test", block_table, &error);
    TEST_CHECK(block == NULL);
    TEST_CHECK(error.code == SYNTAXDEFINITION_REGEX_ERROR_END);
    SyntaxDefinitionError_Deinit(&error);

    Table_Destroy(block_table);
}

Table *table_from_ini(const char *ini_str) {
    IniParser parser;
    IniParser_Init(&parser);
    IniParser_SetText(&parser, ini_str);
    Table *table = IniParser_Parse(&parser);
    TEST_ASSERT(table != NULL);
    IniParser_Deinit(&parser);
    return table;
}

void test_minimal_definition(void) {
    const char *ini = 
    "[meta]\n"
    "name = MINI\n"
    "[block:root]\n"
    "start=\".*\"\n";
    Table *table = table_from_ini(ini);
    SyntaxDefinitionError error;
    SyntaxDefinition *def = SyntaxDefinition_FromTable(table, &error);
    TEST_CHECK(def != NULL);
    TEST_CHECK(def->name != NULL);
    TEST_CHECK(strcmp(def->name, "MINI") == 0);
    SyntaxDefinitionError_Deinit(&error);
    SyntaxDefinition_Destroy(def);
    Table_Destroy(table);
}

SyntaxDefinitionErrorCode test_error(const char *ini) {
    Table *table = table_from_ini(ini);
    SyntaxDefinitionError error;
    SyntaxDefinition *def = SyntaxDefinition_FromTable(table, &error);
    TEST_ASSERT(def == NULL);
    Table_Destroy(table);
    SyntaxDefinitionErrorCode code = error.code;
    SyntaxDefinitionError_Deinit(&error);
    return code;
}

typedef struct {
    const char *ini;
    SyntaxDefinitionErrorCode error_code;
} error_test_case;

void test_errors(void) {
    error_test_case cases[] = {
        {
            "[met]\n"  // missing meta section
            "name = MINI\n"
            "[block:root]\n"
            "start=\".*\"\n",
            SYNTAXDEFINITION_NO_META
        },
        {
            "[meta]\n"
            "name = MINI\n",
            SYNTAXDEFINITION_NO_ROOT_BLOCK
        },
        {
            "[meta]\n"
            "name = MINI\n"
            "[block:root]\n"
            "start=\".*\"\n"
            "[block:]\n"
            "start=\".*\"\n",
            SYNTAXDEFINITION_BLOCK_NAME_EMPTY
        },

        {
            "[meta]\n"
            "name = MINI\n"
            "[block:root]\n"
            "start=\".*)(\"\n",
            SYNTAXDEFINITION_REGEX_ERROR_START
        },
    };

    int cases_num = sizeof(cases) / sizeof(cases[0]);

    for (int i=0; i<cases_num; i++) {
        SyntaxDefinitionErrorCode error_code = test_error(cases[i].ini);
        TEST_CHECK(error_code == cases[i].error_code);
    }
}

TEST_LIST = {
    { "SyntaxDefinition: Block Simple", test_block_simple },
    { "SyntaxDefinitions: Block with End", test_block_with_end },
    { "SyntaxDefinition: Block Errors", test_block_errors },
    { "SyntaxDefinition: Minimal Example", test_minimal_definition },
    { "SyntaxDefinition: Errors", test_errors },
    { NULL, NULL }
};
