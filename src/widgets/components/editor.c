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
#include "editor.h"

#include <ctype.h>
#include "display/canvas.h"
#include "io/input.h"
#include "common/logging.h"
#include "common/colors.h"
#include "document/textedit.h"

// timer callback function
static void alternate_cursor_visibility(uint8_t timer_id, void *user_data) {
    Editor *data = (Editor*)user_data;
    data->cursor_visible = !data->cursor_visible;
    Timer_Restart(timer_id);
}

// widget->ops->free() function
static void editor_destroy(Widget *self) {
    Editor *editor = AS_EDITOR(self);
    Timer_Stop(editor->cursor_timer);
    TextLayout_Deinit(&editor->tl);
    TextEdit_Deinit(&editor->te);
    TextSelection_Deinit(&editor->ts);
}


void draw_char(TextLayout *tl, Canvas *canvas, UTF8Char ch, int x_pos, bool has_cursor) {
    Style orig_style;
    if (has_cursor) {
        orig_style = canvas->current_style;
        canvas->current_style.attributes |= STYLE_UNDERLINE;
    }
    if (UTF8_IsASCII(ch) && UTF8_AsASCII(ch) == '\t') {
        int tabwidth = TextLayout_CalcTabWidth(tl, x_pos);
        for (int i=0; i<tabwidth; i++) {
            Canvas_PutChar(canvas, utf8_space);
        }
    }
    else {
        Canvas_PutChar(canvas, ch);
    }
    if (has_cursor) {
        canvas->current_style = orig_style;
    }
}

// draw line and return new y_offset (changes to one if the cursor wraps to the next line)
static int draw_visual_line(Editor *editor, Canvas *canvas, int y, int y_offset) {
    VisualLine *line = TextLayout_GetVisualLine(&editor->tl, y);
    TextSelection *ts = &editor->ts;

    if (!line){
        return y_offset;
    }

    // Move cursor to the start position
    Canvas_MoveCursor(canvas, 0, y + y_offset);

    // get information about the cursor
    CursorLayoutInfo cursor;
    TextLayout_GetCursorLayoutInfo(&editor->tl, &cursor);

    // change the style if it's the current line
    Style orig_style = canvas->current_style;
    uint8_t line_style_color;
    if (line->src == editor->tb->current_line) {
        line_style_color = Color_GetCodeById(COLOR_HIGHLIGHT_BG);
    }
    else {
        line_style_color = orig_style.bg;
    }

    // draw the characters
    for (int i=0; i<line->length; i++) {
        UTF8Char ch = VisualLine_GetChar(line, i);
        bool has_cursor = (line == cursor.line && i == cursor.idx && editor->cursor_visible);
        if (TextSelection_IsSelected(ts, line->src, line->offset + i)) {
            canvas->current_style.bg = Color_GetCodeById(COLOR_SECONDARY_BG);
        }
        else {
            canvas->current_style.bg = line_style_color;
        }
        draw_char(&editor->tl, canvas, ch, line->char_x[i], has_cursor);
    }

    canvas->current_style.bg = line_style_color;

 
    int x = line->width;

    // if the cursor is behind the last character of the line draw it
    bool has_cursor = (line == cursor.line && line->length == cursor.idx);
    if (has_cursor) {
        // if it goes out of screen wrap the line first
        if (x == editor->tl.width) {
            y_offset++;
            x = 0;
            Canvas_MoveCursor(canvas, 0, y + y_offset);
        }
        draw_char(&editor->tl, canvas, utf8_space, x,  editor->cursor_visible);
        x++;
    }

    // fill the line with spaces (to overwrite artifacts from earlier draws)
    for ( ; x<editor->tl.width; x++) {
        draw_char(&editor->tl, canvas, utf8_space, x, false);
    }

    // restore the previous style
    canvas->current_style = orig_style;

    return y_offset;
}

// widget->ops->draw() function
static void editor_draw(const Widget *self, Canvas *canvas) {
    Editor *editor = AS_EDITOR(self);
    CursorLayoutInfo cursor;
    TextLayout_GetCursorLayoutInfo(&editor->tl, &cursor);
    
    int y = 0;
    int y_offset = 0;
    for (y=0; y+y_offset<editor->tl.height; y++) {
        y_offset = draw_visual_line(editor, canvas, y, y_offset);
    }
}

static void scroll_down(TextLayout *tl, TextEdit *te) {
    if (TextLayout_AtBottom(tl)) {
        return;
    }
    CursorLayoutInfo cursor;
    TextLayout_GetCursorLayoutInfo(tl, &cursor);
    if (cursor.y == 0) {
        TextEdit_MoveDown(te);
    }
    TextLayout_ScrollDown(tl);
}

static void scroll_up(TextLayout *tl, TextEdit *te) {
    if (TextLayout_AtTop(tl)) {
        return;
    }
    CursorLayoutInfo info;
    TextLayout_GetCursorLayoutInfo(tl, &info);
    if (info.y == tl->height - 1) {
        TextEdit_MoveUp(te);
    }
    TextLayout_ScrollUp(tl);
}

// widget->ops->on_input() function
static bool editor_handle_input(Widget *self, EscapeSequence key, UTF8Char ch) {
    (void)key;
    (void)self;
    TextLayout *tl = &AS_EDITOR(self)->tl;
    TextEdit *te = &AS_EDITOR(self)->te;
    TextSelection *ts = &AS_EDITOR(self)->ts;

    CursorLayoutInfo cursor;
    TextLayout_GetCursorLayoutInfo(tl, &cursor);

    // No Input
    if ((UTF8_Equal(ch, utf8_invalid) && key == ESC_NONE)) {
        return false;
    }

    // Selection
    if (key == ESC_SHIFT_CURSOR_LEFT || key == ESC_SHIFT_CURSOR_RIGHT || key == ESC_SHIFT_CURSOR_UP || key == ESC_SHIFT_CURSOR_DOWN) {
        TextBuffer_MergeGap(te->tb);
        TextSelection_Select(ts, cursor.line->src, cursor.line->offset + cursor.idx);
        switch (key) {
            case ESC_SHIFT_CURSOR_LEFT:
                TextEdit_MoveLeft(te);
                break;
            case ESC_SHIFT_CURSOR_RIGHT:
                TextEdit_MoveRight(te);
                break;
            case ESC_SHIFT_CURSOR_UP:
                TextEdit_MoveUp(te);
                break;
            case ESC_SHIFT_CURSOR_DOWN:
                TextEdit_MoveDown(te);
                break;
            default:
                break;
        }
        TextBuffer_MergeGap(te->tb);
        TextLayout_GetCursorLayoutInfo(tl, &cursor);
        TextSelection_Select(ts, cursor.line->src, cursor.line->offset + cursor.idx);
        return true;
    }
    else {
        TextSelection_Abort(ts);
    }

    // Everything else
    if (ch.length == 1) {
        char c = ch.bytes[0];
        if (c == KEY_ENTER) {
            TextEdit_Newline(te);
            return true;
        }
        else if (c == KEY_BACKSPACE) {
            TextEdit_Backspace(te);
            return true;
        }
        else if (c == KEY_TAB) {
            TextEdit_InsertChar(te, UTF8_GetCharFromString("\t"));
            return true;
        }
    }
    if (UTF8_IsPrintable(ch)) {
        TextEdit_InsertChar(te, ch);
        return true;
    }
    else if (key == ESC_DELETE) {
        TextEdit_DeleteChar(te);
        return true;
    }
    else if (key == ESC_CURSOR_LEFT) {
        TextEdit_MoveLeft(te);
        return true;
    }
    else if (key == ESC_CURSOR_RIGHT) {
        TextEdit_MoveRight(te);
        return true;
    }
    else if (key == ESC_CURSOR_UP) {
        TextEdit_MoveUp(te);
        return true;
    }
    else if (key == ESC_CURSOR_DOWN) {
        TextEdit_MoveDown(te);
        return true;
    }
    else if (key == ESC_HOME) {
        return true;
    } 
    else if (key == ESC_END) {
        return true;
    }
    else if (key == ESC_PAGE_DOWN) {
        for (int i=0; i<5; i++) {
            scroll_down(tl, te);
        }
        return true;
    }
    else if (key == ESC_PAGE_UP) {
        for (int i=0; i<5; i++) {
            scroll_up(tl, te);
        }
        return true;
    }
    else if (key == ESC_SHIFT_PAGE_DOWN) {
        return true;
    }
    else if (key == ESC_SHIFT_PAGE_UP) {
        
        return true;
    }
    return false;
}

// widget->ops->on_resize() function
static void editor_handle_resize(Widget *self, int parent_w, int parent_h) {
    self->width = parent_w;
    self->height = parent_h - 1;
    TextLayout_SetDimensions(&AS_EDITOR(self)->tl, self->width, self->height);
}

static void editor_on_focus(Widget *self) {
    Timer_Resume(AS_EDITOR(self)->cursor_timer);
}

static void editor_on_blur(Widget *self) {
    Editor *data = AS_EDITOR(self);
    Timer_Pause(data->cursor_timer);
    data->cursor_visible = false;
}

static WidgetOps editor_ops = {
    .destroy = editor_destroy,
    .draw = editor_draw,
    .on_input = editor_handle_input,
    .on_resize = editor_handle_resize,
    .on_focus = editor_on_focus,
    .on_blur = editor_on_blur,
};


void Editor_Init(Editor *self, Widget *parent, TextBuffer *tb) {
    Widget_Init(&self->base, parent, &editor_ops);
    self->tb = tb;
    
    TextLayout_Init(&self->tl, tb,self->base.width, self->base.height);
    TextEdit_Init(&self->te, tb, &self->tl);
    TextSelection_Init(&self->ts);

    self->cursor_timer = Timer_Start(500, alternate_cursor_visibility, self);
    Timer_Pause(self->cursor_timer);  // start when getting focus
    self->cursor_visible = true;

    self->base.style.bg = Color_GetCodeById(COLOR_BG);
    self->base.style.fg = Color_GetCodeById(COLOR_FG);
}

Editor *Editor_Create(Widget *parent, TextBuffer *tb) {
    Editor *new = malloc(sizeof(Editor));
    if (!new) {
        logFatal("Cannot allocate memory for Editor.");
    }
    Editor_Init(new, parent, tb);
    return new;
}