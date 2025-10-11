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

    if (!line){
        return y_offset;
    }

    Canvas_MoveCursor(canvas, 0, y + y_offset);

    CursorLayoutInfo cursor;
    TextLayout_GetCursorLayoutInfo(&editor->tl, &cursor);

    Style orig_style = canvas->current_style;;
    if (line->src == editor->tb->current_line) {
        canvas->current_style.bg = Color_GetCodeById(COLOR_HIGHLIGHT_BG);
    }

    for (int i=0; i<line->length; i++) {
        UTF8Char ch = VisualLine_GetChar(line, i);
        bool has_cursor = (line == cursor.line && i == cursor.idx && editor->cursor_visible);
        draw_char(&editor->tl, canvas, ch, line->char_x[i], has_cursor);
    }

    int x = line->width;

    bool has_cursor = (line == cursor.line && line->length == cursor.idx);
    if (has_cursor) {
        if (x == editor->tl.width) {
            y_offset++;
            x = 0;
            Canvas_MoveCursor(canvas, 0, y + y_offset);
        }
        draw_char(&editor->tl, canvas, utf8_space, x,  editor->cursor_visible);
        x++;
    }
    for ( ; x<editor->tl.width; x++) {
        draw_char(&editor->tl, canvas, utf8_space, x, false);
    }

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

// widget->ops->on_input() function
static bool editor_handle_input(Widget *self, EscapeSequence key, UTF8Char ch) {
    (void)key;
    (void)self;
    TextLayout *tl = &AS_EDITOR(self)->tl;
    TextEdit *te = &AS_EDITOR(self)->te;

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
        TextLayout_ScrollDown(tl);
        return true;
    }
    else if (key == ESC_PAGE_UP) {
        TextLayout_ScrollUp(tl);
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