#include "editor.h"

#include <ctype.h>
#include "display/canvas.h"
#include "io/input.h"
#include "common/logging.h"
#include "document/textedit.h"
#include "document/textcursor.h"

static void alternate_cursor_visibility(uint8_t timer_id, void *user_data) {
    EditorData *data = (EditorData*)user_data;
    data->cursor_visible = !data->cursor_visible;
    Timer_Restart(timer_id);
}

// widget->ops->free() function
static void editor_Destroy(Widget *self) {
    Timer_Stop(((EditorData*)self->data)->cursor_timer);
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

static void fill_line_with_spaces(const Widget *self, Canvas *canvas, int *x, int *y) {
    int n = self->width  - (*x);
    draw_spaces(self, canvas, n, x, y);
}

static void draw_line(const Widget *self, const UTF8String *text, Canvas *canvas, int *y) {
    int x = 0;
    draw_text(self, text, canvas, &x, y);
    fill_line_with_spaces(self, canvas, &x, y);
}

static void draw_current_line(const Widget *self, const TextBuffer *tb, Canvas *canvas, int *y) {
    int x = 0;
    EditorData *data = (EditorData*)self->data;
    UTF8String cursor;
    UTF8String_Init(&cursor);
    if (data->cursor_visible) {
        UTF8String_FromStr(&cursor, "_", 1);
    }
    else {
        UTF8String_FromStr(&cursor, " ", 1);
    }
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

// widget->ops->draw() function
static void editor_draw(const Widget *self, Canvas *canvas) {
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

static void editor_scroll_up(EditorData *data) {
    if (data->first_line->prev == NULL) {
        return;
    }
    data->first_line = data->first_line->prev;
}

static void editor_scroll_down(EditorData *data) {
    if (data->first_line->next == NULL) {
        return;
    }
    data->first_line = data->first_line->next;
}

// widget->ops->handle_input() function
static bool editor_handle_input(Widget *self, EscapeSequence key, UTF8Char ch) {
    (void)key;
    EditorData *data = (EditorData*)self->data;

    if (ch.length == 1) {
        char c = ch.bytes[0];
        if (c == KEY_ENTER) {
            TB_Enter(data->tb);
            return true;
        }
        else if (c == KEY_BACKSPACE) {
            TB_Backspace(data->tb);
            return true;
        }
        else if (c == KEY_TAB) {
            return true;
        }
    }
    if (UTF8_IsPrintable(ch)) {
        TB_InsertChar(data->tb, ch);
    }
    else if (key == ESC_CURSOR_LEFT) {
        TB_MoveCursor(data->tb, -1);
    }
    else if (key == ESC_CURSOR_RIGHT) {
        TB_MoveCursor(data->tb, 1);
    }
    else if (key == ESC_CURSOR_UP) {
        TB_ChangeLine(data->tb, -1);
    }
    else if (key == ESC_CURSOR_DOWN) {
        TB_ChangeLine(data->tb, 1);
    }
    else if (key == ESC_HOME) {
        TB_Home(data->tb);
    } 
    else if (key == ESC_END) {
        TB_End(data->tb);
    }
    else if (key == ESC_PAGE_DOWN) {
        editor_scroll_down(data);
    }
    else if (key == ESC_PAGE_UP) {
        editor_scroll_up(data);
    }
    else if (key == ESC_SHIFT_PAGE_DOWN) {
        editor_scroll_down(data);
        editor_scroll_down(data);
        editor_scroll_down(data);
        editor_scroll_down(data);
        editor_scroll_down(data);
    }
    else if (key == ESC_SHIFT_PAGE_UP) {
        editor_scroll_up(data);
        editor_scroll_up(data);
        editor_scroll_up(data);
        editor_scroll_up(data);
        editor_scroll_up(data);
    }
    return true;
}

// widget->ops->handle_resize() function
static void editor_handle_resize(Widget *self, int parent_w, int parent_h) {
    self->width = parent_w;
    self->height = parent_h - 1;
}

static WidgetOps editor_ops = {
    .destroy = editor_Destroy,
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
    data->cursor_timer = Timer_Start(500, alternate_cursor_visibility, data);
    data->cursor_visible = true;
    Widget *new = Widget_Create(parent, &editor_ops);
    new->data = data;

    new->style.bg = 232;
    return new;
}