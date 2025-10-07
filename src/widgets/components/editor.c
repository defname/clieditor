#include "editor.h"

#include <ctype.h>
#include "display/canvas.h"
#include "io/input.h"
#include "common/logging.h"
#include "common/colors.h"
#include "document/textedit.h"

static void alternate_cursor_visibility(uint8_t timer_id, void *user_data) {
    Editor *data = (Editor*)user_data;
    data->cursor_visible = !data->cursor_visible;
    Timer_Restart(timer_id);
}

// widget->ops->free() function
static void editor_Destroy(Widget *self) {
    Timer_Stop(AS_EDITOR(self)->cursor_timer);
}

static void draw_text(const UTF8String *text, Canvas *canvas, int start, int length) {
    for (int i=start; i<start + length; i++) {
        Canvas_PutChar(canvas, text->chars[i]);
    }
}

static void draw_cursor(Editor *editor, Canvas *canvas) {
    UTF8String cursor;
    UTF8String_Init(&cursor);
    char c = ' ';
    if (editor->cursor_visible) {
        c = '|';
    }
    UTF8String_FromStr(&cursor, &c, 1);
    draw_text(&cursor, canvas, 0, 1);
    UTF8String_Deinit(&cursor);
}

static void draw_spaces(Canvas *canvas, int n) {
    if (n <= 0) {
        return;
    }
    UTF8String s;
    UTF8String_Init(&s);
    UTF8String_Spaces(&s, n);
    draw_text(&s, canvas, 0, n);
    UTF8String_Deinit(&s);
}

// widget->ops->draw() function
static void editor_draw(const Widget *self, Canvas *canvas) {
    Editor *editor = AS_EDITOR(self);

    int cursor_y = TextLayout_GetCursorY(&editor->tl);
    int cursor_x = TextLayout_GetCursorX(&editor->tl);

    int y = 0;
    for (y=0; y<editor->tl.height; y++) {
        VisualLine *line = TextLayout_GetVisualLine(&editor->tl, y);
        int line_length = TextLayout_GetVisualLineLength(&editor->tl, y);
        Canvas_MoveCursor(canvas, 0, y);
        if (!line){
            break;
        }
        if (line->src == editor->tb->current_line) {
            Style s = canvas->current_style;
            canvas->current_style.bg = Color_GetCodeById(COLOR_HIGHLIGHT_BG);
            if (cursor_y == y) {
                draw_text(&line->src->text, canvas, line->offset, cursor_x);
                draw_cursor(editor, canvas);
                draw_text(&line->src->text, canvas, cursor_x, line_length - cursor_x);
                draw_spaces(canvas, editor->tl.width - line_length);
            }
            else {
                draw_text(&line->src->text, canvas, line->offset, line_length);
                draw_spaces(canvas, editor->tl.width - line_length);
            }
            canvas->current_style.bg = s.bg;
        }
        else {
            draw_text(&line->src->text, canvas, line->offset, line_length);
            draw_spaces(canvas, editor->tl.width - line_length);
        }
    }
}

// widget->ops->on_input() function
static bool editor_handle_input(Widget *self, EscapeSequence key, UTF8Char ch) {
    (void)key;
    (void)self;
    //TextLayout *tl = &AS_EDITOR(self)->tl;
    TextEdit *te = &AS_EDITOR(self)->te;

    if (ch.length == 1) {
        char c = ch.bytes[0];
        if (c == KEY_ENTER) {
            return true;
        }
        else if (c == KEY_BACKSPACE) {
            return true;
        }
        else if (c == KEY_TAB) {
            return true;
        }
    }
    if (UTF8_IsPrintable(ch)) {
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
        return true;
    }
    else if (key == ESC_PAGE_UP) {
        
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
    .destroy = editor_Destroy,
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