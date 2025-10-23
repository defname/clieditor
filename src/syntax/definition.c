/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "definition.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "common/logging.h"
#include "common/typedtable.h"
#include "common/tableiterator.h"


#define NO_ERROR ((SyntaxDefinitionError){ .code = SYNTAXDEFINITION_NO_ERROR, .message = (String){0} })
#define ERROR(err_code, ...) ((SyntaxDefinitionError){ .code = err_code, .message = String_Format(__VA_ARGS__) })

static void set_error(SyntaxDefinitionError *error, SyntaxDefinitionErrorCode code, String message) {
    error->code = code;
    logError("Error in SyntaxDefinition: %s", String_AsCStr(&message));
    String_Take(&error->message, &message);
}


void SyntaxDefinitionError_Deinit(SyntaxDefinitionError *error) {
    String_Deinit(&error->message);
}


SyntaxBlockDef *SyntaxBlockDef_Create() {
    SyntaxBlockDef *block = malloc(sizeof(SyntaxBlockDef));
    if (!block) {
        logFatal("Cannot allocate memory for SyntaxBlockDef.");
    }
    block->name = NULL;
    block->only_start = false;
    block->children = NULL;
    block->children_count = 0;
    return block;
}


void SyntaxBlockDef_Destroy(SyntaxBlockDef *block) {
    if (block->children) {
        free(block->children);
    }
    if (block->name) {
        free(block->name);
    }
    regfree(&block->start);
    regfree(&block->end);

    free(block);
}


SyntaxBlockDef *SyntaxBlockDef_FromTable(const char *name, const Table *table, SyntaxDefinitionError *error) {
    *error = NO_ERROR;
    const char *start_regex = TypedTable_GetString(table, "start");
    const char *end_regex = TypedTable_GetString(table, "end");

    SyntaxBlockDef *block = SyntaxBlockDef_Create();
    block->name = strdup(name);
    block->only_start = true;
    block->color = (uint8_t)TypedTable_GetNumber(table, "color");

    if (strcmp(name, "root") == 0) {
        regcomp(&block->start, "^", REG_EXTENDED);  // matches always without consuming
        regcomp(&block->end, "a^", REG_EXTENDED);   // matches never
        block->only_start = false;
        return block;
    }
    
    if (!start_regex) {
        set_error(error,
            SYNTAXDEFINITION_BLOCK_NO_START_REGEX, 
            String_Format("Block \"%s\" has no start regex defined.", name)
        );
        free(block->name);
        free(block);
        return NULL;
    }
    int ret = regcomp(&block->start, start_regex, REG_EXTENDED);
    if (ret != 0) {
        char errbuf[256];
        regerror(ret, &block->start, errbuf, sizeof(errbuf));
        set_error(error,
            SYNTAXDEFINITION_REGEX_ERROR_START, 
            String_Format("Error in start regex \"%s\" in block \"%s\": %s", start_regex, name, errbuf)
        );
        free(block->name);
        free(block);
        return NULL;
    }
    if (end_regex) {
        ret = regcomp(&block->end, end_regex, REG_EXTENDED);
        if (ret != 0) {
            char errbuf[256];
            regerror(ret, &block->end, errbuf, sizeof(errbuf));
            set_error(error,
                SYNTAXDEFINITION_REGEX_ERROR_END, 
                String_Format("Error in end regex \"%s\" in block \"%s\": %s", end_regex, name, errbuf)
            );
            regfree(&block->start);
            free(block->name);
            free(block);
            return NULL;
        }
        block->only_start = false;
    }
    else {
        regcomp(&block->end, "^", REG_EXTENDED);  // matches everything without consuming
    }

    return block;
}


// helper struct
typedef struct _table_block_mapping {
    const Table *table;
    SyntaxBlockDef *block;
} table_block_mapping;


static SyntaxDefinitionError init_definition(SyntaxDefinition *def, const Table *table) {
    def->name = NULL;
    def->blocks = NULL;
    def->blocks_count = 0;
    def->root = NULL;

    // Get meta table
    Table *meta = TypedTable_GetTable(table, "meta");
    if (!meta) {
        return ERROR(SYNTAXDEFINITION_NO_META, "No meta section found.");
    }
    const char *name = TypedTable_GetString(meta, "name");
    def->name = strdup(name ?  name : "");

    // initialize block list
    def->blocks = malloc(sizeof(SyntaxBlockDef*) * (Table_GetUsage(table) - 1));
    if (!def->blocks) {
        logFatal("Cannot allocate memory for SyntaxDefinition blocks");
    }

    return NO_ERROR;
}


static SyntaxDefinitionError build_blocks(SyntaxDefinition *def, const Table *table, Table *blocks) {
    // get all block tables and build all block definitions (without children)
    TableIterator it = TableIterator_Begin(table);
    while (TableIterator_Next(&it)) {
        const char *key = it.current->key;

        if (strncmp(key, "block:", 6) == 0) {
            const char *block_name = key + 6;
            const TypedValue *value =  (TypedValue*)it.current->value;
            if (value->type != VALUE_TYPE_TABLE) {
                // block is no block, but a value or something
                return ERROR(SYNTAXDEFINITION_BLOCK_NOT_A_SECTION, "Block \"block:%s\" is not a section.", block_name);
            }
            if (strlen(block_name) == 0) {
                // block name is empty
                return ERROR(SYNTAXDEFINITION_BLOCK_NAME_EMPTY, "Empty block name found.");
            }
            const Table *block_table = value->data.table_value;

            // Create the block (without linking children)
            SyntaxDefinitionError error;
            SyntaxBlockDef *block = SyntaxBlockDef_FromTable(block_name, block_table, &error);

            if (!block) {
                // block definition was invalid
                return error;
            }
            SyntaxDefinitionError_Deinit(&error);

            // set the block in the temporary table
            table_block_mapping *mapping = malloc(sizeof(table_block_mapping));
            if (!mapping) {
                logFatal("Cannot allocate memory for table_block_mapping.");
            }
            mapping->table = block_table;
            mapping->block = block;
            Table_Set(blocks, block_name, mapping, free);

            // add the block to the list of all blocks (important for freeing also unreferenced blocks later)
            def->blocks[def->blocks_count++] = block;

            if (strcmp(block_name, "root") == 0) {
                def->root = block;
            }
        }
    }

    if (!def->root) {
        return ERROR(SYNTAXDEFINITION_NO_ROOT_BLOCK, "No root block defined.");
    }

    return NO_ERROR;
}


SyntaxDefinitionError link_children(SyntaxDefinition *def, Table *blocks) {
    for (size_t i=0; i<def->blocks_count; i++) {
        SyntaxBlockDef *block = def->blocks[i];
        
        // get the mapping
        table_block_mapping *mapping = Table_Get(blocks, block->name);
        if (!mapping) {
            logFatal("Some serious design flaw detected.");
        }
        const Table *block_table = mapping->table;

        const char *children_str = TypedTable_GetString(block_table, "allowed_blocks");
        if (!children_str) {
            continue;
        }

        String s = String_FromCStr(children_str, strlen(children_str));
        ssize_t children_count = 0;
        String delimiter = String_FromCStr(",", 1);
        StringView *children = String_Split(&s, &delimiter, &children_count);
        if (children_count < 0) {
            logFatal("Potential rrror in String_Split().");
        }
        String_Deinit(&delimiter);
        if (children_count == 0) {
            String_Deinit(&s);
            free(children);
            continue;
        }

        // malloc memory for the children of SyntaxBlockDef
        block->children = malloc(sizeof(SyntaxBlockDef*) * children_count);
        if (!block->children) {
            logFatal("Cannot allocate memory for children of SyntaxBlockDef.");
        }

        for (size_t child_idx=0; child_idx<(size_t)children_count; child_idx++) {
            String child_name = String_FromView(children[child_idx]);
            String_Trim(&child_name);
            table_block_mapping *child_mapping = Table_Get(blocks, child_name.bytes);
            
            if (!child_mapping) {
                // no block with the given child name
                // something like
                // allowed_blocks = block1, non_existing_block
                SyntaxDefinitionError error = ERROR(
                    SYNTAXDEFINITION_BLOCK_DOES_NOT_EXIST,
                    "Block \"%s\" defines non-existing block \"%s\" as it's child.", block->name, child_name.bytes
                );
                String_Deinit(&child_name);
                String_Deinit(&s);
                free(children);
                return error;
            }
            String_Deinit(&child_name);
            block->children[child_idx] = child_mapping->block;
        }
        block->children_count = children_count;
        String_Deinit(&s);
        free(children);
    }
    return NO_ERROR;
}


SyntaxDefinition *SyntaxDefinition_FromTable(const Table *table, SyntaxDefinitionError *error) {
    if (!table) {
        return NULL;
    }

    SyntaxDefinition *def = malloc(sizeof(SyntaxDefinition));
    if (!def) {
        logFatal("Cannot allocate memory for SyntaxDefinition.");
    }

    // Initialize SyntaxDefinition instance
    *error = init_definition(def, table);
    if (error->code != SYNTAXDEFINITION_NO_ERROR) {
        SyntaxDefinition_Destroy(def);
        return NULL;
    }
    SyntaxDefinitionError_Deinit(error);

    // temporary table mapping all block names to it's SyntaxBlockDef
    Table *blocks = Table_Create();
    *error = build_blocks(def, table, blocks);
    if (error->code != SYNTAXDEFINITION_NO_ERROR) {
        SyntaxDefinition_Destroy(def);
        Table_Destroy(blocks);
        return NULL;
    }
    SyntaxDefinitionError_Deinit(error);

    // construct the children lists
    *error = link_children(def, blocks);
    if (error->code != SYNTAXDEFINITION_NO_ERROR) {
        SyntaxDefinition_Destroy(def);
        Table_Destroy(blocks);
        return NULL;
    }
    SyntaxDefinitionError_Deinit(error);

    // cleanup
    Table_Destroy(blocks);

    return def;
}

void SyntaxDefinition_Destroy(SyntaxDefinition *def) {
    if (!def) {
        return;
    }
    if (def->name) {
        free(def->name);
    }
    if (def->blocks) {
        for (size_t i=0; i<def->blocks_count; i++) {
            SyntaxBlockDef_Destroy(def->blocks[i]);
        }
        free(def->blocks);
    }
    free(def);
}


#undef NO_ERROR
#undef ERROR