#include "textlayoutbindings.h"

void SyntaxHighlightingBinding_Init(SyntaxHighlightingBinding *binding, TextLayout *tl, SyntaxHighlighting *sh) {
    binding->tl = tl;
    binding->sh = sh;
    binding->need_full_update = true;
}

void SyntaxHighlightingBinding_Deinit(SyntaxHighlightingBinding *binding) {
    binding->tl = NULL;
    binding->sh = NULL;
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

static void update_following_lines(SyntaxHighlightingBinding *binding, const Line *first_line, const Line *last_line, Stack *open_blocks) {
    const Stack *open_blocks_begin = open_blocks;
    const Line *line = first_line;
    while (line) {
        if (line != first_line) {
            SyntaxHighlightingString *shs = Table_Get(binding->sh->strings, &line->text);
            if (shs && stack_equal(&shs->open_blocks_at_begin, open_blocks_begin)) {
                break;
            }
        }
        // open_blocks == NULL is also handled by the function
        const Stack *open_blocks_end = SyntaxHighlighting_HighlightString(binding->sh, &line->text, open_blocks_begin);
        open_blocks_begin = open_blocks_end;
        //Stack_Destroy(open_blocks_end);
        if (line == last_line) {
            break;
        }
        line = line->next;
    }
}

void SyntaxHighlightingBinding_UpdateLine(SyntaxHighlightingBinding *binding, const Line *line, const Line *last_line) {
    if (!binding || !binding->sh || !binding->tl || !binding->tl->tb || !line) {
        return;
    }

    if (!last_line) {
        last_line = line;
    }

    Line *prev_line = line->prev;
    Stack *open_blocks = NULL;
    if (prev_line) {
        // check if highlighting for the previous line is already calculated
        SyntaxHighlightingString *shs = Table_Get(binding->sh->strings, &prev_line->text);
        if (shs) {
            // use the open_blocks from the end of previous line
            open_blocks = &shs->open_blocks_at_end;
        }
        else {
            // highlighting for the line is not calculated so far
            // so run this function for thr previous line
            SyntaxHighlightingBinding_UpdateLine(binding, prev_line, last_line);
            return;
        }
    }
    if (line == binding->tl->tb->current_line) {
        // this is super dirty cause the gap funcitonality is totally disabled this way!!!
        // NEED A BETTER SOLUTION
        TextBuffer_MergeGap((TextBuffer*)binding->tl->tb);
    }

    // update all lines until last_line (including)
    update_following_lines(binding, line, last_line, open_blocks);
    if (open_blocks) {
        //Stack_Destroy(open_blocks);
    }
}

void SyntaxHighlightingBinding_Update(SyntaxHighlightingBinding *binding) {
    if (!binding || !binding->sh || !binding->tl || !binding->tl->tb) {
        return;
    }
    const TextBuffer *tb = binding->tl->tb;
    Line *current = tb->current_line;
    VisualLine *last_vl = TextLayout_GetVisualLine(binding->tl, binding->tl->height - 1);
    Line *last = last_vl ? last_vl->src : TextBuffer_GetLastLine(tb);
    if (!current || !last) {
        return;
    }
    SyntaxHighlightingBinding_UpdateLine(binding, current, last);
}

void SyntaxHighlightingBinding_UpdateAll(SyntaxHighlightingBinding *binding, bool force) {
    if (!binding || !binding->tl || !binding->tl->tb) {
        return;
    }
    if (!force && !binding->need_full_update) {
        return;
    }
    binding->need_full_update = false;
    SyntaxHighlightingBinding_UpdateLine(
        binding,
        TextBuffer_GetFirstLine(binding->tl->tb),
        TextBuffer_GetLastLine(binding->tl->tb)
    );

}