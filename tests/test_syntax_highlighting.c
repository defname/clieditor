#include "acutest.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "syntax/highlighting.h"
#include "syntax/definition.h"
#include "common/iniparser.h"

static SyntaxDefinition *create_definition(const char *ini) {
    IniParser parser;
    IniParser_Init(&parser);
    IniParser_SetText(&parser, ini);
    Table *table = IniParser_Parse(&parser);
    TEST_ASSERT(table != NULL);
    SyntaxDefinitionError error;
    SyntaxDefinition *def = SyntaxDefinition_FromTable(table, &error);
    TEST_ASSERT(def != NULL);
    IniParser_Deinit(&parser);
    Table_Destroy(table);
    return def;
}

Table *build_blocks_table(SyntaxDefinition *def) {
    Table *table = Table_Create();
    for (size_t i=0; i<def->blocks_count; i++) {
        SyntaxBlockDef *block = def->blocks[i];
        Table_Set(table, block->name, block, NULL);
    }
    return table;
}

typedef struct {
    const char *ini;
    const char *str;
    size_t tags_count;
    size_t tag_offsets[32];
    const char *tag_blocks[32];
    size_t open_blocks_count;
    const char *open_blocks[32];
} TagTestCase;

static void assert_highlight_tags(
    TagTestCase testcase
) {
    const char *ini = testcase.ini;
    String str = String_Format(testcase.str);
    size_t tags_count = testcase.tags_count;
    size_t *tag_offsets = testcase.tag_offsets;
    const char **tag_blocks = testcase.tag_blocks;
    size_t open_blocks_count_expected = testcase.open_blocks_count;
    const char **open_blocks_expected = testcase.open_blocks;


    SyntaxDefinition *def = create_definition(ini);
    SyntaxHighlighting hl;
    SyntaxHighlighting_Init(&hl, def);

    const Stack *open_blocks = SyntaxHighlighting_HighlightString(&hl, &str, NULL);

    SyntaxHighlightingString *shs = Table_Get(hl.strings, &str);
    TEST_ASSERT(open_blocks == &shs->open_blocks_at_end);

    TEST_CHECK(!Stack_IsEmpty(open_blocks));
    TEST_MSG("Expected open_blocks to not be not empty.");

    for (size_t i=0; i<open_blocks_count_expected; i++) {
        const SyntaxBlockDef *block = Stack_Pop(open_blocks);
        TEST_CHECK(block != NULL);
        TEST_CHECK(strcmp(open_blocks_expected[i], block->name) == 0);
        TEST_MSG("Expected open block #%zu to be '%s' but got '%s'.", i, open_blocks_expected[i], block->name);
    }
    
    TEST_CHECK(shs->tags_count == tags_count);
    TEST_MSG("Expected %zu tags but got %zu.", tags_count, shs->tags_count);

    for (size_t i=0; i<tags_count; i++) {
        TEST_CHECK(shs->tags[i].byte_offset == tag_offsets[i]);
        TEST_MSG("Expected offset #%zu to be %zu but got %zu.", i, tag_offsets[i], shs->tags[i].byte_offset);

        TEST_CHECK(strcmp(shs->tags[i].block->name, tag_blocks[i]) == 0);
        TEST_MSG("Expected block #%zu to be '%s' but got '%s'.", i, tag_blocks[i], shs->tags[i].block->name);
    }

    String_Deinit(&str);
    SyntaxHighlighting_Deinit(&hl);
    SyntaxDefinition_Destroy(def);
}

const char *test_ini0 = 
"[meta]\n"
"name = TEST\n"
"[block:root]\n"
"start=^\n"
"end=a^\n"
"child_blocks=string\n"
"\n"
"[block:string]\n"
"start=\"'\"\n"
"end=\"'\"\n";


void test_highlight_string_simple(void) {
    SyntaxDefinition *def = create_definition(test_ini0);
    SyntaxHighlighting hl;
    SyntaxHighlighting_Init(&hl, def);

    String test1 = String_Format("root 'string' root");
    Stack *open_blocks_at_begin = Stack_Create();
    Stack_Push(open_blocks_at_begin, def->root);
    Stack *open_blocks = SyntaxHighlighting_HighlightString(&hl, &test1, open_blocks_at_begin);

    TEST_CHECK(open_blocks != NULL);
    TEST_CHECK(Stack_Peek(open_blocks) == def->root);
    SyntaxHighlightingString *shs = Table_Get(hl.strings, &test1);
    TEST_CHECK(shs->tags_count == 2);
    TEST_CHECK(shs->tags[0].byte_offset == 5);  // start of 'string'
    TEST_CHECK(strcmp(shs->tags[0].block->name, "string") == 0);
    TEST_CHECK(shs->tags[1].byte_offset == 13);  // end of 'string'
    TEST_CHECK(shs->tags[1].block == def->root);

    Stack_Destroy(open_blocks_at_begin);
    String_Deinit(&test1);
    SyntaxHighlighting_Deinit(&hl);
    SyntaxDefinition_Destroy(def);
}

const char *test_ini1 = R"(
[meta]
name = TEST

[block:root]
child_blocks=string, comment, keyword, brackets

[block:brackets]
start=\(
end=\)
child_blocks=string, keyword, brackets

[block:keyword]
start=keyword

[block:string]
start='
end='

[block:comment]
start = //
end = $
)";


void test_basics(void) {
    TagTestCase cases[] = {
        {
            test_ini1,
            "foobar // blabla ' bla",  // comment
            2,
            {7, 22},
            {"comment", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "foo 'bar' foo",  // string
            2,
            {4, 9},
            {"string", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "foo keyword foo",  // keyword
            2,
            {4, 11},
            {"keyword", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "foo '//not a comment' foo",  // string with comment inside (should be ignored)
            2,
            {4, 21},
            {"string", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "foo 'not a keyword' foo",  // keyword inside string
            2,
            {4, 19},
            {"string", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "foo (no brackets) foo",
            2,
            {4, 17},
            {"brackets", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "()",
            2,
            {0, 2},
            {"brackets", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "(keyword)keyword",
            6,
            {0, 1, 8, 9, 9, 16},
            {"brackets", "keyword", "brackets", "root", "keyword", "root"},
            1,
            {"root"}
        },
        {
            test_ini1,
            "",
            0,
            {},
            {},
            1,
            {"root"}
        },
        {
            test_ini1,
            "(('not a keyword'))",
            6,
            {0, 1, 2, 17, 18, 19},
            {"brackets", "brackets", "string", "brackets", "brackets", "root"},
            1,
            {"root"}
        },
    };
    size_t count = sizeof(cases) / sizeof(cases[0]);

    for (size_t i=0; i<count; i++) {
        TEST_CASE(cases[i].str);
        assert_highlight_tags(cases[i]);
    }
}


const char *test_ini2 = R"(
[meta]
name = TEST2

[block:root]
child_blocks=assignment, comment

[block:assignment]
start=^[a-zA-Z_:]+=
end=$
child_blocks=value

[block:value]
start=.
end=$
ends_on=comment

[block:comment]
start = //
end = $
)";

void test_moderate(void) {
    TagTestCase cases[] = {
        {
            test_ini2,
            "foo=blub // should end assignment",  // assignment with comment
            5,
            {0, 4, 9, 33, 33},
            {"assignment", "value", "comment", "assignment", "root"},
            1,
            {"root"}
        },
    };
    size_t count = sizeof(cases) / sizeof(cases[0]);

    for (size_t i=0; i<count; i++) {
        TEST_CASE(cases[i].str);
        assert_highlight_tags(cases[i]);
    }
}


void test_open_blocks(void) {
    TagTestCase cases[] = {
        {
            test_ini1,
            "(keyword",
            3,
            {0, 1, 8},
            {"brackets", "keyword", "brackets"},
            2,
            {"brackets", "root"}
        },

    };
    size_t count = sizeof(cases) / sizeof(cases[0]);

    for (size_t i=0; i<count; i++) {
        TEST_CASE(cases[i].str);
        assert_highlight_tags(cases[i]);
    }
}

/****************************************************************/
/* Random Tests                                                 */
char random_char() {
    char c = 0;
    for (;;) {
        c = rand() % 128;
        if (isprint(c) && c != '\r' && c != '\n') {
            return c;
        }
    }
}

void generate_random_string(char *out, size_t max_len, const char *tokens[], size_t token_count) {
    //size_t len = rand() % (max_len/2) + (max_len/2) - 1;
    size_t len = rand() % max_len;
    size_t l = 0;
    int token_prop = rand() % 10 + 1;
    while (l < len) {
        int choose_token = rand() % token_prop;
        if (choose_token == 0) {
            size_t i = rand() % (token_count);
            const char *token = tokens[i];
            size_t token_len = strlen(token);
            if (l + token_len >= max_len) {
                break;
            }
            memcpy(out + l, token, token_len);
            l += token_len;
        }
        else {
            out[l++] = random_char();
        }
    }
    out[l] = '\0';
}


void test_stress(void) {
    // function should just not crash on random input
    const char *tokens[] = {
        "(", ")", "keyword", "'", "//"
    };

    srand(time(NULL));

    for (int i=0; i<1000; i++) {
        SyntaxDefinition *def = create_definition(test_ini1);
        SyntaxHighlighting hl;
        SyntaxHighlighting_Init(&hl, def);
        
        char str[4096];
        generate_random_string(str, 1024, tokens, sizeof(tokens) / sizeof(tokens[0]));
        TEST_CASE(str);

        String test = String_FromCStr(str, strlen(str));
        Stack *open_blocks = SyntaxHighlighting_HighlightString(&hl, &test, NULL);

        TEST_CHECK(open_blocks != NULL);
        TEST_CHECK(!Stack_IsEmpty(open_blocks));

        String_Deinit(&test);
        SyntaxHighlighting_Deinit(&hl);
        SyntaxDefinition_Destroy(def);
    }

}

TEST_LIST = {
    { "SyntaxHighlighting: Simple", test_highlight_string_simple },
    { "SyntaxHighlighting: Basics", test_basics },
    { "SyntaxHighlighting: Moderate", test_moderate },
    { "SyntaxHighlighting: Open blocks", test_open_blocks },
    { "SyntaxHighlighting: Random Tests", test_stress },
    { NULL, NULL }
};