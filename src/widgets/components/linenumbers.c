#include "linenumbers.h"
#include "common/logging.h"
#include "common/utf8string.h"
#include "common/colors.h"



static void draw(const Widget *self, Canvas *canvas) {
    (void)self;
    (void)canvas;
    LineNumbers *ln = AS_LINENUMBERS(self);
    
    UTF8String str;
    UTF8String_Init(&str);

    UTF8Char border_char = UTF8_GetCharFromString("│");

    int line_nr = ln->first_number;
    int width = self->width;

    uint8_t color = canvas->current_style.fg;
    
    for (int i=0; i<ln->tl->height; i++) {
        VisualLine *line = TextLayout_GetVisualLine(ln->tl, i);
        if (!line) {
            break;
        }
        VisualLine *prev = TextLayout_GetVisualLine(ln->tl, i-1);
        if (!prev || prev->src != line->src) {
            
            if (line->src == ln->tl->tb->current_line) {
                canvas->current_style.fg = Color_GetCodeById(COLOR_FG);
            }
            Canvas_MoveCursor(canvas, 0, i);
            UTF8String_Format(&str, 10, "%*d", width-1, line_nr);
            Canvas_Write(canvas, &str);
            if (canvas->current_style.fg != color) {
                canvas->current_style.fg = color;
            }
            Canvas_PutChar(canvas, border_char);
            line_nr++;
        }
        else {
            Canvas_MoveCursor(canvas, width-1, i);
            Canvas_PutChar(canvas, border_char);
        }
    }
    UTF8String_Deinit(&str);
}

WidgetOps linenumber_ops = {
    .draw = draw,
};

void LineNumbers_Init(LineNumbers *ln, Widget *parent, TextLayout *tl) {
    Widget *w = AS_WIDGET(ln);
    Widget_Init(w, parent, &linenumber_ops);
    ln->tl = tl;

    w->width = 4;
    
    w->style.bg = Color_GetCodeById(COLOR_BG);
    w->style.fg = Color_GetCodeById(COLOR_HIGHLIGHT_BG);
}

LineNumbers *LineNumbers_Create(Widget *parent, TextLayout *tl) {
    LineNumbers *new = malloc(sizeof(LineNumbers));
    if (!new) {
        logFatal("Cannot allocate memory for LineNumbers.");
    }
    LineNumbers_Init(new, parent, tl);
    return new;
}
