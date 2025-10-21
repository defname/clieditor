#include "definition.h"
#include "common/logging.h"
#include "common/typedtable.h"
#include "common/tableiterator.h"

static SyntaxBlockDef *SyntaxBlockDef_Create() {
    SyntaxBlockDef *block = malloc(sizeof(SyntaxBlockDef));
    if (!block) {
        logFatal("Cannot allocate memory for SyntaxBlockDef.");
    }
    block->name[0] = '\0';
    block->only_start = false;
    block->children = NULL;
    block->children_count = 0;
    return block;
}

static void SyntaxBlockDef_Destroy(SyntaxBlockDef *block) {
    free(block->children);
    free(block);
}
static SyntaxBlockDef *SyntaxBlockDef_FromTable(const char *name, const Table *table) {

}

SyntaxDefinition *SyntaxDefinition_FromTable(const Table *table) {
    SyntaxDefinition *def = malloc(sizeof(SyntaxDefinition));
    if (!def) {
        logFatal("Cannot allocate memory for SyntaxDefinition.");
    }
    Table *meta = TypedTable_GetTable(table, "meta");
    if (!meta) {
        return NULL;
    }
    def->name = TypedTable_GetString(meta, "name");

    TableIterator it = TableIterator_Begin(table);
    while (!TableIterator_IsEnd(it)) {
        const char *key = TableIterator_Key(&it);
        if (strncmp(key, "block:") == 0) {
            const char *block_name = key + 6;
            SyntaxBlockDef *block = SyntaxBlockDef_FromTable(block_name, TableIterator_Value(&it));
            if (!block) {
                SyntaxDefinition_Destroy(def);
                return NULL;
            }
        }

        TableIterator_Next(&it);

}