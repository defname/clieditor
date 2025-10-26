#ifndef SYNTAX_TEXTLAYOUTBINDINGS_H
#define SYNTAX_TEXTLAYOUTBINDINGS_H

#include <stdbool.h>
#include "highlighting.h"
#include "document/textlayout.h"

typedef struct _SyntaxHighlightingBinding {
    TextLayout *tl;
    SyntaxHighlighting *sh;
    bool need_full_update;
} SyntaxHighlightingBinding;

void SyntaxHighlightingBinding_Init(SyntaxHighlightingBinding *binding, TextLayout *tl, SyntaxHighlighting *sh);
void SyntaxHighlightingBinding_Deinit(SyntaxHighlightingBinding *binding);

void SyntaxHighlightingBinding_UpdateLine(SyntaxHighlightingBinding *binding, const Line *line, const Line *last_line);
void SyntaxHighlightingBinding_Update(SyntaxHighlightingBinding *binding);
void SyntaxHighlightingBinding_UpdateAll(SyntaxHighlightingBinding *binding, bool force);

#endif