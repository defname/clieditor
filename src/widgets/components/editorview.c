#include "editorview.h"
#include "common/logging.h"



static int get_first_line_number(TextLayout *tl) {
    Line *line = tl->first_line;
    int i = 1;
    while (line->prev) {
        i++;
        line = line->prev;
    }
    return i;
}

static int get_max_width(int max_nr) {
    // simple solution without logarithms
    int width = 1;
    while (max_nr >= 10) {
        max_nr /= 10;
        width++;
    }
    return width;
}


static void update(Widget *w) {
    EditorView *ev = AS_EDITOR_VIEW(w);
    TextLayout *tl = &ev->editor->tl;

    int first_line_number = get_first_line_number(tl);
    ev->line_numbers->first_number = first_line_number;
    
    int max_line_number = first_line_number + tl->height - 1;  // could be less if lines are wrapped
    int max_width = get_max_width(max_line_number);

    if (max_width + 1 == AS_WIDGET(ev->line_numbers)->width) {  // nothing changed...
        return;
    }

    AS_WIDGET(ev->line_numbers)->width = max_width + 1;
    Editor_Resize(ev->editor, w->width - max_width - 1, w->height);
    AS_WIDGET(ev->editor)->x = max_width + 1;
}

static void on_resize(Widget *w, int parent_w, int parent_h) {
    EditorView *ev = AS_EDITOR_VIEW(w);
    w->width = parent_w;
    w->height = parent_h - 1;
    AS_WIDGET(ev->line_numbers)->height = w->height;
    AS_WIDGET(ev->line_numbers)->width = 4;
    AS_WIDGET(ev->editor)->x = 4;
    Editor_Resize(ev->editor, w->width - 4, w->height);
}

static void on_focus(Widget *w) {
    Widget_Focus(AS_WIDGET(AS_EDITOR_VIEW(w)->editor));
}

WidgetOps editorview_ops = {
    .on_resize = on_resize,
    .on_focus = on_focus,
    .update = update,
};

void EditorView_Init(EditorView *self, Widget *parent, TextBuffer *tb) {
    Widget_Init(&self->base, parent, &editorview_ops);
    self->editor = Editor_Create(AS_WIDGET(self), tb);
    self->line_numbers = LineNumbers_Create(AS_WIDGET(self), &self->editor->tl);
}

EditorView *EditorView_Create(Widget *parent, TextBuffer *tb) {
    EditorView *new = malloc(sizeof(EditorView));
    if (!new) {
        logFatal("Cannot allocate memory for EditorView.");
    }
    EditorView_Init(new, parent, tb);
    return new;
}
