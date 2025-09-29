#include "editor.h"

#include <ctype.h>
#include "core/canvas.h"
#include "core/input.h"
#include "utils/logging.h"

void editor_free(Widget *self) {
    free(self->data);
}

static void draw_text(const Widget *self, const UTF8String *text, Canvas *canvas, int *x, int *y) {
    for (size_t i=0; i<text->length; i++) {
        if ((*x) >= self->width) {  // line wrap
            (*x) = 0;
            (*y)++;
            if ((*y) >= self->height) {
                return;
            }
            Canvas_MoveCursor(canvas, *x, *y);
        }
        Canvas_PutChar(canvas, text->chars[i]);
        (*x)++;
    }
}

static void draw_spaces(const Widget *self, Canvas *canvas, int n, int *x, int *y) {
    if (n <= 0) {
        return;
    }
    UTF8String s;
    UTF8String_Init(&s);
    UTF8String_Spaces(&s, n);
    draw_text(self, &s, canvas, x, y);
    UTF8String_Deinit(&s);
}

void fill_line_with_spaces(const Widget *self, Canvas *canvas, int *x, int *y) {
    int n = self->width  - (*x) - 1;
    draw_spaces(self, canvas, n, x, y);
}

static void draw_line(const Widget *self, const UTF8String *text, Canvas *canvas, int *y) {
    int x = 0;
    draw_text(self, text, canvas, &x, y);
    fill_line_with_spaces(self, canvas, &x, y);
}

static void draw_current_line(const Widget *self, const TextBuffer *tb, Canvas *canvas, int *y) {
    int x = 0;
    UTF8String cursor;
    UTF8String_Init(&cursor);
    UTF8String_FromStr(&cursor, "_", 1);
    UTF8String before, after;
    UTF8String_Init(&before);
    UTF8String_Init(&after);
    TB_TextAroundGap(tb, &before, &after);
    draw_text(self, &before, canvas, &x, y);
    draw_text(self, &tb->gap.text, canvas, &x, y);
    draw_text(self,&cursor, canvas, &x, y);
    draw_text(self, &after, canvas, &x, y);
    UTF8String_Deinit(&before);
    UTF8String_Deinit(&after);
    UTF8String_Deinit(&cursor);
    fill_line_with_spaces(self, canvas, &x, y);
}

void editor_draw(const Widget *self, Canvas *canvas) {
    EditorData *data = (EditorData*)self->data;
    Line *current = data->first_line;
    int y = 0;
    while ((y < self->height) && current) {
        UTF8String *text = &current->text;
        if (current == data->tb->current_line) {
            Style s = canvas->current_style;
            canvas->current_style.bg = 237;
            draw_current_line(self, data->tb, canvas, &y);
            canvas->current_style.bg = s.bg;
        }
        else {
            draw_line(self, text, canvas, &y);
        }
        y++;
        Canvas_MoveCursor(canvas, 0, y);
        if (y >= self->height) {
            return;
        }
        current = current->next;
    }
}

bool editor_handle_input(Widget *self, EscapeSequence key, UTF8Char ch) {
    (void)key;
    EditorData *data = (EditorData*)self->data;

    if (ch.length == 1) {
        char c = ch.bytes[0];
        if (c == KEY_ENTER) {
            TB_Enter(data->tb);
        }
        else if (c == KEY_BACKSPACE) {
            TB_Backspace(data->tb);
        }
        else if (c == KEY_TAB) {
            
        }
    }
    if (UTF8_IsPrintable(ch)) {
        TB_InsertChar(data->tb, ch);
    }
    if (key == ESC_CURSOR_LEFT) {
        TB_MoveCursor(data->tb, -1);
    }
    if (key == ESC_CURSOR_RIGHT) {
        TB_MoveCursor(data->tb, 1);
    }
    if (key == ESC_CURSOR_UP) {
        TB_ChangeLine(data->tb, -1);
    }
    if (key == ESC_CURSOR_DOWN) {
        TB_ChangeLine(data->tb, 1);
    }
    return true;
}

void editor_handle_resize(Widget *self, int parent_w, int parent_h) {
    self->width = parent_w;
    self->height = parent_h - 1;
}

static WidgetOps editor_ops = {
    .destroy = editor_free,
    .draw = editor_draw,
    .handle_input = editor_handle_input,
    .handle_resize = editor_handle_resize
};

Widget *Editor_Create(Widget *parent, TextBuffer *tb) {
    EditorData *data = malloc(sizeof(EditorData));
    if (!data) {
        logFatal("Cannot allocate memory for EditorData.");
    }
    data->tb = tb;
    data->first_line = tb->current_line;
    Widget *new = Widget_Create(parent, &editor_ops);
    new->data = data;

    new->style.bg = 232;
    return new;
}