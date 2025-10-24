#include "textlayoutbindings.h"

void SyntaxHighlightingBinding_Init(SyntaxHighlightingBinding *binding, const TextLayout *tl, SyntaxHighlighting *sh) {
    binding->tl = tl;
    binding->sh = sh;
}

void SyntaxHighlightingBinding_Deinit(SyntaxHighlightingBinding *binding) {
    binding->tl = NULL;
    binding->sh = NULL;
}

static void update_lines(SyntaxHighlightingBinding *binding, const Line *line, const Line *last_line, const Stack *open_blocks) {
    const Stack *open_blocks_begin = open_blocks;
    while (line) {
        // open_blocks == NULL is also handled by the function
        const Stack *open_blocks_end = SyntaxHighlighting_HighlightString(binding->sh, &line->text, open_blocks_begin);
        open_blocks_begin = open_blocks_end;
        if (line == last_line) {
            break;
        }
        line = line->next;
    }
}

void SyntaxHighlightingBinding_Update(SyntaxHighlightingBinding *binding, const Line *line, const Line *last_line) {
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
            SyntaxHighlightingBinding_Update(binding, prev_line, last_line);
            return;
        }
    }
    if (line == binding->tl->tb->current_line) {
        // this is super dirty cause the gap funcitonality is totally disabled this way!!!
        // NEED A BETTER SOLUTION
        TextBuffer_MergeGap((TextBuffer*)binding->tl->tb);
    }

    // update all lines until last_line (including)
    update_lines(binding, line, last_line, open_blocks);
}
