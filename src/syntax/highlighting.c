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
    Stack_Init(&shs->open_blocks_at_begin);
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
    Stack_Deinit(&shs->open_blocks_at_begin);
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
    hl->strings = Table_CreatePtr();
}

void SyntaxHighlighting_Deinit(SyntaxHighlighting *hl) {
    if (hl->strings) {
        Table_Destroy(hl->strings);
    }
    hl->strings = NULL;
    hl->def = NULL;
}

static bool regexec_with_cache(const regex_t *regex, const char *str, size_t offset, MatchCache *cache, regmatch_t *match) {
    if (cache->done) {
        return false;
    }
    if (cache->match.rm_so + cache->offset >= (regoff_t)offset) {
        *match = cache->match;
        match->rm_so = cache->match.rm_so + cache->offset - offset;
        match->rm_eo = cache->match.rm_eo + cache->offset - offset;
        return true;
    }
    if (regexec(regex, str + offset, 1, match, 0) == 0) {
        cache->match = *match;
        cache->offset = offset;
        return true;
    }
    cache->done = true;
    return false;
}

static SyntaxBlockDef *find_first_block(SyntaxBlockDef **block_list, size_t block_list_count, const char *str, size_t offset, regmatch_t *match) {
    bool had_match = false;
    regmatch_t first_match = {0};
    SyntaxBlockDef *first_block = NULL;
    for (size_t i=0; i<block_list_count; i++) {
        SyntaxBlockDef *curr = block_list[i];
        regmatch_t curr_match;
        if (regexec_with_cache(&curr->start, str, offset, &curr->start_cache, &curr_match)) {
            if (!had_match || curr_match.rm_so < first_match.rm_so) {
                first_match = curr_match;
                first_block = curr;
                had_match = true;
            }
        }
    }
    if (had_match) {
        *match = first_match;
        return first_block;
    }
    return NULL;
}

static SyntaxBlockDef *find_first_child(const char *str, size_t offset, SyntaxBlockDef *current, regmatch_t *match) {
    return find_first_block(current->children, current->children_count, str, offset, match);
}

static SyntaxBlockDef *find_first_ends_on_block(const char *str, size_t offset, SyntaxBlockDef *current, regmatch_t *match) {
    return find_first_block(current->ends_on, current->ends_on_count, str, offset, match);
}

static bool find_end_of_block(const char *str, size_t offset, SyntaxBlockDef *current, regmatch_t *match) {
    return regexec_with_cache(&current->end, str, offset, &current->end_cache, match);
}


static void init_match_cache(SyntaxHighlighting *sh) {
    for (size_t i=0; i<sh->def->blocks_count; i++) {
        SyntaxBlockDef *block = sh->def->blocks[i];
        block->start_cache.match.rm_so = -1;
        block->start_cache.match.rm_eo = -1;
        block->start_cache.offset = 0;
        block->start_cache.done = false;
        block->end_cache.match.rm_so = -1;
        block->end_cache.match.rm_eo = -1;
        block->end_cache.offset = 0;
        block->end_cache.done = false;

    }
}

static bool stack_equal(const Stack *a, const Stack *b) {
    if (a->size != b->size) {
        return false;
    }
    for (size_t i=0; i<a->size; i++) {
        if (a->items[i] != b->items[i]) {
            return false;
        }
    }
    return true;
}

const Stack *SyntaxHighlighting_HighlightString(SyntaxHighlighting *sh, const String *text, const Stack *open_blocks_at_begin) {
    // check if there is already old infomation about text in the table
    SyntaxHighlightingString *shs = Table_Get(sh->strings, text);
    if (shs) {
        // early exit if nothing changed since the last calculation
        if (stack_equal(&shs->open_blocks_at_begin, open_blocks_at_begin)) {
        return &shs->open_blocks_at_end;
        }
        // if open_blocks_at_begin changed clear the old information
        SyntaxHighlightingString_Clear(shs);
    }
    else {
        // if there is no object present in sh->strings table create one and store it
        shs = SyntaxHighlightingString_Create(text);
        Table_Set(sh->strings, text, shs, (void(*)(void*))SyntaxHighlightingString_Destroy);
    }

    // create a working copy of the stack
    if (open_blocks_at_begin) {
        Stack_CopyTo(&shs->open_blocks_at_begin, open_blocks_at_begin);
        Stack_CopyTo(&shs->open_blocks_at_end, open_blocks_at_begin);
    }
    else {
        // or create a new one if open_blocks_at_begin is NULL
        Stack_Clear(&shs->open_blocks_at_begin);
        Stack_Push(&shs->open_blocks_at_begin, sh->def->root);
        Stack_CopyTo(&shs->open_blocks_at_end, &shs->open_blocks_at_begin);
    }
    Stack *open_blocks = &shs->open_blocks_at_end;


    // initialize the cache fields of the SyntaxBlockDefs
    init_match_cache(sh);

    // iterate over the string
    size_t offset = 0;
    for (;;) {
        // take the current block from the stack (but keep it there)
        SyntaxBlockDef *current_block = (SyntaxBlockDef*)Stack_Peek(open_blocks);

        // find the first child block
        regmatch_t child_match;
        SyntaxBlockDef *child = find_first_child(text->bytes, offset, current_block, &child_match);
        // find the first ends_on block
        regmatch_t ends_on_match;
        SyntaxBlockDef *ends_on = find_first_ends_on_block(text->bytes, offset, current_block, &ends_on_match);

        // find the block_end
        regmatch_t end_match = {0, 0};  // current position with no consumption
        bool end_found = true;   // true for the case current_block is an only start block
        if (!current_block->only_start) {
            end_found = find_end_of_block(text->bytes, offset, current_block, &end_match);
        }

        // check if child is the first match
        if (child
            && (!ends_on || child_match.rm_so < ends_on_match.rm_so)
            && (!end_found || child_match.rm_so < end_match.rm_so))
        {
            // if there is a child block found create and add a tag to SyntaxHighlightingString tag list
            SyntaxHighlightingTag tag;
            tag.text = text;
            tag.byte_offset = offset + child_match.rm_so;
            tag.block = child;
            SyntaxHighlightingString_AddTag(shs, tag);

            // increase the offset to the end of the match
            offset += child_match.rm_eo;

            // push the child to the stack to continue with it in the next iteration
            Stack_Push(open_blocks, (void*)child);
            continue;
        }
        // check if ends_on is the first match
        if (ends_on
            && (!end_found || ends_on_match.rm_so < end_match.rm_so))
        {
            // the current block ends by the occurence of the ends_on block
            Stack_Pop(open_blocks);  // removes current (which was just peeked before)

            // add a tag for the beginning of the ends_on block
            SyntaxHighlightingTag tag;
            tag.text = text;
            tag.byte_offset = offset + ends_on_match.rm_so;
            tag.block = ends_on;
            SyntaxHighlightingString_AddTag(shs, tag);

            // add the ends_on block to the stack
            Stack_Push(open_blocks, (void*)ends_on);

            // increase the offset to the end of the match
            offset += ends_on_match.rm_eo;
            continue;
        }
        // last case.... if end_found it's the first match automatically
        if (end_found) {
            // end of block found so remove it from stack
            Stack_Pop(open_blocks);  // removes current (which was just peeked before)

            // and add a tag for the (new) begin of the surrounding block
            SyntaxHighlightingTag tag;
            tag.text = text;
            tag.byte_offset = offset + end_match.rm_eo;  // the current block goes behind the match of its end
            tag.block = Stack_Peek(open_blocks);  // new current block
            SyntaxHighlightingString_AddTag(shs, tag);

            // increase offset
            offset += end_match.rm_eo;
            continue;
        }
        // neither the beginning of a new block nor the end of the current block found
        return open_blocks;
    }

    // if this happens there is an error in the SyntaxDefinition.
    // the end pattern of the root block must *never* match
    return NULL;
}
