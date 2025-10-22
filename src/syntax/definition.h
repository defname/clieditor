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

 /**
 * @file definition.h
 * @brief Module for creating and managing SyntaxDefinition instances.
 *
 * This module constructs `SyntaxDefinition` objects from the output of the
 * IniParser module.
 *
 * The INI file must contain a `[meta]` section and one or more block sections
 * named `block:<block_name>`. At least one block named `root` must exist.
 *
 * ### Minimal example
 * ```
 * [meta]
 * name = TEST
 * 
 * [block:root]
 * start = .
 * ```
 *
 * Each block must define a `start` regex (an extended POSIX regular expression).
 * Optionally, a block may also define an `end` regex.
 *
 * Blocks without an `end` expression represent single tokens or patterns that
 * cannot contain nested elements (for example, keywords or inline constructs).
 *
 * Blocks that define an `end` expression may additionally define a list of
 * `allowed_blocks`, which specifies which block types are permitted to appear
 * inside the block.
 */

#ifndef SYNTAX_DEFINITION_H
#define SYNTAX_DEFINITION_H

#include <stdint.h>
#include <regex.h>
#include "common/table.h"
#include "common/string.h"


typedef enum {
    SYNTAXDEFINITION_NO_ERROR,
    SYNTAXDEFINITION_BLOCK_NO_START_REGEX,
    SYNTAXDEFINITION_REGEX_ERROR_START,
    SYNTAXDEFINITION_REGEX_ERROR_END,
    SYNTAXDEFINITION_NO_META,
    SYNTAXDEFINITION_BLOCK_NOT_A_SECTION,
    SYNTAXDEFINITION_BLOCK_NAME_EMPTY,
    SYNTAXDEFINITION_NO_ROOT_BLOCK,
    SYNTAXDEFINITION_BLOCK_DOES_NOT_EXIST
} SyntaxDefinitionErrorCode;

/**
 * @brief Holds error information. 
 */
typedef struct _SyntaxDefinitionError {
    SyntaxDefinitionErrorCode code;
    String message;
} SyntaxDefinitionError;

/**
 * @brief Deinitialize an error.
 * 
 * This only needs to be called if `SyntaxDefinition_FromTable()` returns `NULL`.
 */
void SyntaxDefinitionError_Deinit(SyntaxDefinitionError *error);


/**
 * @brief Holds the definition of a syntax block.
 */
typedef struct _SyntaxBlockDef {
    char *name;      //< name of the block definition. for easier debugging
    regex_t start;      //< compiled regex to determine the begin of the block
    regex_t end;        //< compiled regex to determine the end of the block (optional)
    
    bool only_start;    //< if true only start is tested and no children are allowed
    
    struct _SyntaxBlockDef **children;  //< list of children definition which are allowed inside the block
    size_t children_count;              //< number of children
    
    uint8_t color;      //< the color to render the block
} SyntaxBlockDef;

SyntaxBlockDef *SyntaxBlockDef_Create();
void SyntaxBlockDef_Destroy(SyntaxBlockDef *block);
SyntaxBlockDef *SyntaxBlockDef_FromTable(const char *name, const Table *table, SyntaxDefinitionError *error);


/**
 * @brief Holds a complete syntax definition.
 */
typedef struct _SyntaxDefinition {
    char *name;                 //< name of the filetype (e.g. INI)
    SyntaxBlockDef *root;       //< root block definition
    SyntaxBlockDef **blocks;    //< list of all blocks
    size_t blocks_count;         //< number of blocks
} SyntaxDefinition;

 
/**
 * @brief Constructs a SyntaxDefinition from a typed Table.
 * 
 * @param table A typed Table (see common/typedtable.h) holding the syntax definition (should be the result of `IniParser`).
 * @param error A pointer to an uninitialized `SyntaxDefinitionError` strcut.
 * 
 * @returns
 * A SyntaxDefinition or NULL if the definition in table is not valid.
 * 
 * If an error occures `error`contains an error code and a message. In the case of an error
 * `error` need to be deinitialized with `SyntaxDefinitionError_Deinit()`.
 */
SyntaxDefinition *SyntaxDefinition_FromTable(const Table *table, SyntaxDefinitionError *error);

/**
 * @brief Deinitialize and destroy def.
 */
void SyntaxDefinition_Destroy(SyntaxDefinition *def);

#endif