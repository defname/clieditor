#ifndef SYNTAX_TEXTLAYOUTBINDINGS_H
#define SYNTAX_TEXTLAYOUTBINDINGS_H

#include "highlighting.h"
#include "document/textlayout.h"

typedef struct _SyntaxHighlightingBinding {
    TextLayout *tl;
    SyntaxHighlighting *sh;
} SyntaxHighlightingBinding;

void SyntaxHighlightingBinding_Init(SyntaxHighlightingBinding *binding, TextLayout *tl, SyntaxHighlighting *sh);
void SyntaxHighlightingBinding_Deinit(SyntaxHighlightingBinding *binding);

void SyntaxHighlightingBinding_UpdateLine(SyntaxHighlightingBinding *binding, const Line *line, const Line *last_line);
void SyntaxHighlightingBinding_Update(SyntaxHighlightingBinding *binding);

#endif