#ifndef SYNTAX_TEXTLAYOUTBINDINGS_H
#define SYNTAX_TEXTLAYOUTBINDINGS_H

#include "highlighting.h"
#include "document/textlayout.h"

typedef struct _SyntaxHighlightingBinding {
    const TextLayout *tl;
    SyntaxHighlighting *sh;
} SyntaxHighlightingBinding;

void SyntaxHighlightingBinding_Init(SyntaxHighlightingBinding *binding, const TextLayout *tl, SyntaxHighlighting *sh);
void SyntaxHighlightingBinding_Deinit(SyntaxHighlightingBinding *binding);

void SyntaxHighlightingBinding_Update(SyntaxHighlightingBinding *binding, const Line *line, const Line *last_line);

#endif