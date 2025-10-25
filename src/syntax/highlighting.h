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
 * @file highlighting.h
 * @brief System to store the information about the highlighting, like text positions and colors.
 * 
 * ### Limitations
 * Regular expressions that mark the begin or the end of an block are not tested across different
 * Strings. If for example a keyword starts in one string and continues in another one, even if
 * they are parsed in the correct order, the keyword will not be recognized.
 * So make sure the `text` parts are  sperarated in a compatible way (like by newlines or similar).
 */
#ifndef SYNTAX_HIGHLIGHTING_H
#define SYNTAX_HIGHLIGHTING_H

#include <stddef.h>
#include "definition.h"
#include "common/stack.h"
#include "common/table.h"
#include "common/string.h"

/**
 * @brief Holds information about the beginning of a block.
 * 
 * Instead of keeping track of the end of a block it is just treated
 * as a new beginning of the surrounding block.
 * 
 * Note that this struct does not have the ownership of any of the members.
 */
typedef struct _SyntaxHighlightingTag {
    const String *text;             //< pointer to the string this tag is for
    size_t byte_offset;             //< byte offset where the tag occurs
    const SyntaxBlockDef *block;    //< pointer to the block this tag is for
} SyntaxHighlightingTag;

void SyntaxHighlightingTag_Init(SyntaxHighlightingTag *tag);
void SyntaxHighlightingTag_Deinit(SyntaxHighlightingTag *tag);


/**
 * @brief Holds the complete highlighting information for the String `text`.
 */
typedef struct _SyntaxHighlightingString {
    const String *text;             //< pointer to the text information are stored for
    SyntaxHighlightingTag *tags;    //< list of tags for `text`
    size_t tags_count;              //< number of tags in `tags`
    size_t tags_capacity;           //< capacity of `tags`

    Stack open_blocks_at_begin;     //< Stack of (SyntaxBlockDef*) elements that are open
    Stack open_blocks_at_end;       //< Stack of (SyntaxBlockDef*) elements that are open at the end of `text`
} SyntaxHighlightingString;

#define SHS_TAGS_INITIAL_CAPACITY 16
#define SHS_TAGS_GROW_FACTOR 2

SyntaxHighlightingString *SyntaxHighlightingString_Create(const String *text);
void SyntaxHighlightingString_Destroy(SyntaxHighlightingString *shs);

void SyntaxHighlightingString_AddTag(SyntaxHighlightingString *shs, SyntaxHighlightingTag tag);
void SyntaxHighlightingString_Clear(SyntaxHighlightingString *shs);

/**
 * @brief Holds the highlighting information for text of multiple `Strings`.
 */
typedef struct _SyntaxHighlighting {
    const SyntaxDefinition *def;    //< SyntaxDefinition to use for highlighting 
    Table *strings;                 //< Table of (String -> SyntaxHighlightingString*) elements (holds the ownership of the SyntaxHighlightingString's)
} SyntaxHighlighting;


/** @brief Initialize syntax highlighting using def for definitions. */
void SyntaxHighlighting_Init(SyntaxHighlighting *sh, const SyntaxDefinition *def);

/** @brief Deinitialize `sh`. */
void SyntaxHighlighting_Deinit(SyntaxHighlighting *sh);

/** @brief Instantiate `SyntaxHighlighting`. */
SyntaxHighlighting *SyntaxHighlighting_Create(const SyntaxDefinition *def);

/** @brief Destroy `sh` */
void SyntaxHighlighting_Destroy(SyntaxHighlighting *sh);

/**
 * @brief Highlight a string according to given context.
 * 
 * Add a `SyntaxHighlightingString`to `hl->strings` or update an existing one.
 * The table uses the pointer address of text as key, so make sure it does not change.
 * open_blocks should only be NULL for the first line. In any other case if should be set to
 * SyntaxHighlighlightingString->open_blocks_at_end of the previous line/text.
 * 
 * @param hl The `SyntaxHighlighting` instance to use.
 * @param text The text to hightlight.
 * @param open_blocks An pointer to a `Stack` instance that holds the open blocks at the beginning of string. If NULL it's assumed that the root block is current.
 * 
 * @returns
 * A reference to the `Stack` containing all open blocks at the end of `text`. 
 */
const Stack *SyntaxHighlighting_HighlightString(SyntaxHighlighting *sh, const String *text, const Stack *open_blocks);

#endif