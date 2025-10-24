#include "acutest.h"
#include "syntax/textlayoutbindings.h"
#include "syntax/definition.h"
#include "common/iniparser.h"
#include "common/string.h"
#include "document/textlayout.h"

typedef struct {
    SyntaxDefinition *def;
    const Line **lines;
    size_t line_count;
    SyntaxHighlighting sh;
    TextBuffer tb;
    TextLayout tl;
    SyntaxHighlightingBinding binding;
} TestFixture;

static void init_textbuffer(TextBuffer *tb, const char *lines[], size_t line_count) {
    TextBuffer_Init(tb);
    TEST_ASSERT(line_count > 0);
    String s = String_FromCStr(lines[0], strlen(lines[0]));
    String_Take(&tb->current_line->text, &s);
    for (size_t i=1; i<line_count; i++) {
        Line *new_line = Line_Create();
        String s = String_FromCStr(lines[i], strlen(lines[i]));
        String_Take(&new_line->text, &s);
        TextBuffer_InsertLineAtBottom(tb, new_line);
    }
    TEST_CHECK(tb->line_count == line_count);
    TEST_MSG("%zu", tb->line_count);
}


static SyntaxDefinition *create_syntax_definition(const char *ini) {
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

static void setup_lines(TestFixture *fixture) {
    fixture->line_count = fixture->tb.line_count;
    fixture->lines = malloc(sizeof(Line*) * fixture->line_count);
    TEST_ASSERT(fixture->lines);
    Line *line = TextBuffer_GetFirstLine(&fixture->tb);
    size_t i = 0;
    while (line) {
        fixture->lines[i++] = line;
        line = line->next;
    }
}

static void setup_fixture(TestFixture *fixture, const char *ini, const char *lines[], size_t line_count) {
    init_textbuffer(&fixture->tb, lines, line_count);
    TextLayout_Init(&fixture->tl, &fixture->tb, 10, 4);
    fixture->def = create_syntax_definition(ini);
    SyntaxHighlighting_Init(&fixture->sh, fixture->def);
    SyntaxHighlightingBinding_Init(&fixture->binding, &fixture->tl, &fixture->sh);
    setup_lines(fixture);
}

static void cleanup_fixture(TestFixture *fixture) {
    SyntaxHighlightingBinding_Deinit(&fixture->binding);
    SyntaxHighlighting_Deinit(&fixture->sh);
    TextLayout_Deinit(&fixture->tl);
    TextBuffer_Deinit(&fixture->tb);
    SyntaxDefinition_Destroy(fixture->def);
    free(fixture->lines);
}




const char *test_ini = R"(
[meta]
name = TEST

[block:root]
child_blocks=keyword, comment, string

[block:keyword]
start = keyword

[block:comment]
start = "#"
end = $

[block:string]
start = "'"
end = "'"
)";

void test_textlayout_bindings(void) {
    // data
    const char *lines[] = {
        "First line",
        "Second line",
    };
    size_t lines_count = sizeof(lines) / sizeof(char*);
    
    // 1. Setup
    TestFixture fixture;
    setup_fixture(&fixture, test_ini, lines, lines_count);
    TextBuffer *tb = &fixture.tb;
    SyntaxHighlighting *sh = &fixture.sh;
    SyntaxHighlightingBinding *binding = &fixture.binding;

    
    // 2. Calculate
    SyntaxHighlightingBinding_Update(binding, tb->current_line->next, tb->current_line->next);

    // 3. Check
    TEST_CHECK(Table_Get(sh->strings, &fixture.lines[0]->text) != NULL);
    TEST_CHECK(Table_Get(sh->strings, &fixture.lines[1]->text) != NULL);

    // 4. Cleanup
    cleanup_fixture(&fixture);
}

TEST_LIST = {
    { "TextLayoutBindings: No Styling", test_textlayout_bindings },
    { NULL, NULL }
};
