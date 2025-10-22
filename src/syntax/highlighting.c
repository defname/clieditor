#include "highlighting.h"
#include "common/logging.h"

/*****************************************************************************/
/* SyntaxHighlightingTag                                                     */

void SyntaxHighlightingTag_Init(SyntaxHighlightingTag *tag) {
    tag->text = NULL;
    tag->byte_offset = 0;
    tag->block = NULL;
}

void SyntaxHighlightingTag_Deinit(SyntaxHighlightingTag *tag) {
    SyntaxHighlightingTag_Init(tag);
}

/*****************************************************************************/
/* SyntaxHighlightingString                                                  */

SyntaxHighlightingString *SyntaxHighlightingString_Create(const String *text) {
    SyntaxHighlightingString *shs = malloc(sizeof(SyntaxHighlightingString));
    if (!shs) {
        logFatal("Cannot allocate memory for SyntaxHighlightingString.");
    }
    shs->text = text;
    shs->tags = NULL;
    shs->tags_count = 0;
    shs->tags_capacity = 0;
    Stack_Init(&shs->open_blocks_at_end);

    shs->tags = malloc(sizeof(SyntaxHighlightingTag) * SHS_TAGS_INITIAL_CAPACITY);
    if (!shs->tags) {
        logFatal("Cannot allocate memory for SyntaxHighlightingString tags.");
    }

    return shs;
}

void SyntaxHighlightingString_Destroy(SyntaxHighlightingString *shs) {
    if (shs->tags) {
        free(shs->tags);
    }
    shs->tags = NULL;
    shs->tags_count = 0;
    shs->tags_capacity = 0;
    shs->text = NULL;
    Stack_Deinit(&shs->open_blocks_at_end);
    free(shs);
}

static void increase_tags_capacity(SyntaxHighlightingString *shs) {
    if (!shs) {
        return;
    }
    if (shs->tags_capacity == 0) {
        shs->tags_capacity = SHS_TAGS_INITIAL_CAPACITY;
    }
    else {
        shs->tags_capacity *= SHS_TAGS_GROW_FACTOR;
    }
    shs->tags = realloc(shs->tags, shs->tags_capacity * sizeof(SyntaxHighlightingTag));
    if (!shs->tags) {
        logFatal("Failed to reallocate SyntaxHighlightingString tags.");
    }
}

void SyntaxHighlightingString_AddTag(SyntaxHighlightingString *shs, SyntaxHighlightingTag tag) {
    if (!shs) {
        return;
    }
    if (shs->tags_count + 1 >= shs->tags_capacity) {
        increase_tags_capacity(shs);
    }
    shs->tags[shs->tags_count++] = tag;
}

void SyntaxHighlightingString_Clear(SyntaxHighlightingString *shs) {
    if (!shs) {
        return;
    }
    shs->tags_count = 0;
    Stack_Clear(&shs->open_blocks_at_end);
}


/*****************************************************************************/
/* SyntaxHighlighting                                                        */

void SyntaxHighlighting_Init(SyntaxHighlighting *hl, const SyntaxDefinition *def) {
    hl->def = def;
    hl->strings = Table_Create();
}

void SyntaxHighlighting_Deinit(SyntaxHighlighting *hl) {
    if (hl->strings) {
        Table_Destroy(hl->strings);
    }
    hl->strings = NULL;
    hl->def = NULL;
}

static const SyntaxBlockDef *find_first_child(const char *str, const SyntaxBlockDef *current, regmatch_t *match) {
    bool had_match = false;
    regmatch_t first_match;
    const SyntaxBlockDef *first_block;
    for (size_t i=0; i<current->children_count; i++) {
        const SyntaxBlockDef *child = current->children[i];
        regmatch_t match;
        if (regexec(&child->start, str, 1, &match, 0) == 0) {
            first_match = match;
            first_block = child;
            had_match = true;
        }
    }
    if (had_match) {
        *match = first_match;
        return first_block;
    }
    return NULL;
}

static bool find_end_of_block(const char *str, const SyntaxBlockDef *current, regmatch_t *match) {
    regmatch_t end_match;
    if (regexec(&current->end, str, 1, &end_match, 0) == 0) {
        *match = end_match;
        return true;
    }
    return false;
}

Stack *SyntaxHighlighting_HighlightString(SyntaxHighlighting *hl, const String *text, const Stack *open_blocks_at_begin) {
    // check if there is already old infomation about text in the table
    SyntaxHighlightingString *shs = Table_Get(hl->strings, text);
    if (shs) {
        // clear it if so
        SyntaxHighlightingString_Clear(shs);
    }
    else {
        // otherwise create and store
        shs = SyntaxHighlightingString_Create(text);
        Table_Set(hl->strings, text, shs, SyntaxHighlightingString_Destroy);
    }

    // create a working copy of the stack
    Stack *open_blocks = Stack_Copy(open_blocks_at_begin);

    // iterate over the string
    size_t offset = 0;
    for (;;) {
        // take the current block from the stack (but keep it there)
        const SyntaxBlockDef *current_block = (SyntaxBlockDef*)Stack_Peek(open_blocks);

        // find the first child block
        regmatch_t match;
        const SyntaxBlockDef *next_block = find_first_child(text->bytes + offset, current_block, &match);
        if (next_block) {
            // if there is a child block found create and add a tag to SyntaxHighlightingString tag list
            SyntaxHighlightingTag tag;
            tag.text = text;
            tag.byte_offset = offset + match.rm_so;
            tag.block = next_block;
            SyntaxHighlightingString_AddTag(shs, tag);

            // increase the offset to the end of the match
            offset += match.rm_eo;

            // push the next_block to the stack to continue with it in the next iteration
            Stack_Push(open_blocks, next_block);
            continue;
        }
        // no child block found
        // so check if the last block ends
        if (find_end_of_block(text->bytes + offset, current_block, &match)) {
            // end of block found so remove it from stack
            Stack_Pop(open_blocks);  // removes current (which was just peeked before)

            // and add a tag for the (new) begin of the surrounding block
            SyntaxHighlightingTag tag;
            tag.text = text;
            tag.byte_offset = offset + match.rm_eo;  // the current block goes behind the match of its end
            tag.block = Stack_Peek(open_blocks);  // new current block
            SyntaxHighlightingString_AddTag(shs, tag);

            // increase offset
            offset += match.rm_eo;
            continue;
        }
        // neither the beginning of a new block nor the end of the current block found
        return open_blocks;
    }

    // if this happens there is an error in the SyntaxDefinition.
    // the end pattern of the root block must *never* match
    return NULL;
}
