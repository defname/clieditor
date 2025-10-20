#include "definition.h"
#include "common/logging.h"
#include "common/typedtable.h"

SyntaxBlockDef *SyntaxBlockDef_Create() {
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

void SyntaxBlockDef_Destroy(SyntaxBlockDef *block) {
    free(block->children);
    free(block);
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

    // Need an Iterator for the table
    
}