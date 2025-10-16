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
#include "linenumbers.h"
#include "common/logging.h"
#include "common/utf8string.h"
#include "common/colors.h"
#include "common/config.h"


static void draw(const Widget *self, Canvas *canvas) {
    (void)self;
    (void)canvas;
    LineNumbers *ln = AS_LINENUMBERS(self);
    
    UTF8String str;
    UTF8String_Init(&str);

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
                canvas->current_style.fg = ln->active_color;
            }
            Canvas_MoveCursor(canvas, 0, i);
            UTF8String_Format(&str, 10, "%*d", width-1, line_nr);
            Canvas_Write(canvas, &str);
            if (canvas->current_style.fg != color) {
                canvas->current_style.fg = color;
            }
            Canvas_PutChar(canvas, ln->border_char);
            line_nr++;
        }
        else {
            Canvas_MoveCursor(canvas, width-1, i);
            Canvas_PutChar(canvas, ln->border_char);
        }
    }
    UTF8String_Deinit(&str);
}

void on_config_change(Widget *self) {
    LineNumbers *ln = AS_LINENUMBERS(self);
    Table *conf = Config_GetModuleConfig("linenumbers");
    self->style.bg = Config_GetColor(conf, "bg", ln->base.style.bg);
    self->style.fg = Config_GetColor(conf, "text", ln->base.style.fg);
    ln->border_char = UTF8_GetCharFromString(Config_GetStr(conf, "border_char", "│"));
    ln->active_color = Config_GetColor(conf, "active_color", ln->active_color);
}

WidgetOps linenumber_ops = {
    .draw = draw,
    .on_config_changed = on_config_change,
};

void LineNumbers_Init(LineNumbers *ln, Widget *parent, TextLayout *tl) {
    Widget *w = AS_WIDGET(ln);
    Widget_Init(w, parent, &linenumber_ops);
    ln->tl = tl;

    w->width = 4;
    
    w->style.bg = Color_GetCodeById(COLOR_BG);
    w->style.fg = Color_GetCodeById(COLOR_HIGHLIGHT_BG);
    ln->active_color = Color_GetCodeById(COLOR_FG);
    ln->border_char = UTF8_GetCharFromString("│");
    ln->first_number = 1;
}

LineNumbers *LineNumbers_Create(Widget *parent, TextLayout *tl) {
    LineNumbers *new = malloc(sizeof(LineNumbers));
    if (!new) {
        logFatal("Cannot allocate memory for LineNumbers.");
    }
    LineNumbers_Init(new, parent, tl);
    return new;
}
