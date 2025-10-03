#include "widget.h"

#include <stdlib.h>
#include "common/logging.h"

void Widget_Init(Widget *widget, Widget *parent, WidgetOps *ops) {
    widget->ops = ops;
    widget->data = NULL;

    widget->x = 0;
    widget->y = 0;
    widget->width = 0;
    widget->height = 0;

    widget->z_index = 0;

    widget->children = NULL;
    widget->children_count = 0;
    widget->children_capacity = 0;

    widget->parent = parent;

    if (parent) {
        Widget_AddChild(parent, widget);
        widget->style = parent->style;
    }
    else {
        widget->style = (Style){ .fg = 15, .bg = 0, .attributes = STYLE_NONE };
    }
}

void Widget_Deinit(Widget *widget) {
    widget->ops = NULL;
    widget->data = NULL;
    widget->children_count = 0;
    free(widget->children);
    widget->children = NULL;
    widget->children_capacity = 0;
}

Widget *Widget_Create(Widget *parent, WidgetOps *ops) {
    Widget *widget = malloc(sizeof(Widget));
    Widget_Init(widget, parent, ops);
    return widget;
}

void Widget_Destroy(Widget *self) {
    if (!self) {
        return;
    }
    // 1. Destroy all children recursively
    for (int i=0; i<self->children_count; i++) {
        Widget_Destroy(self->children[i]);
    }
    // 2. Call the widget's specific destructor (if it exists) to free its data
    if (self->ops && self->ops->destroy) {
        self->ops->destroy(self);
    }
    // 3. Deinitialize the base widget (frees the children array)
    Widget_Deinit(self);
    // 4. Free the widget struct itself
    free(self);
}

// increase widget's children capacity and initialize new entries with NULL
static void increase_capacity(Widget *widget) {
    if (!widget) {
        logError("Invalid widget.");
        return;
    }
    size_t new_capacity;
    if (widget->children_capacity == 0) {
        new_capacity = WIDGET_INITIAL_CAPACITY;
    }
    else {
        new_capacity = widget->children_capacity * WIDGET_GROWTH_FACTOR;
    }
    widget->children = realloc(widget->children, new_capacity * sizeof(Widget *));
    if (!widget->children) {
        logFatal("Failed to reallocate widget children.");
    }
    for (size_t i=widget->children_capacity; i<new_capacity; i++) {
        widget->children[i] = NULL;
    }
    widget->children_capacity = new_capacity;
}

static int compare_children_z_index(const void *a, const void *b) {
    Widget *w1 = *(Widget **)a;
    Widget *w2 = *(Widget **)b;
    if (!w1 && !w2) {
        return 0;
    }
    if (!w1) {
        return 1;
    }
    if (!w2) {
        return -1;
    }
    return w1->z_index - w2->z_index;
}

void Widget_SetZIndex(Widget *self, int z_index) {
    self->z_index = z_index;
    if (self->parent) {
        qsort(self->parent->children, self->parent->children_count, sizeof(Widget *), compare_children_z_index);
    }
}

void Widget_AddChild(Widget *parent, Widget *child) {
    if (!parent || !child) {
        logError("Invalid parent or child widget.");
        return;
    }
    if (parent->children_count >= parent->children_capacity) {
        increase_capacity(parent);
    }
    // find free slot
    for (int i=0; i<parent->children_capacity; i++) {
        if (parent->children[i] == NULL) {
            parent->children[i] = child;
            parent->children_count++;
            child->parent = parent;
            break;
        }
    }
}

void Widget_RemoveChild(Widget *parent, Widget *child) {
    if (!parent || !child) {
        logError("Invalid parent or child widget.");
        return;
    }
    for (int i=0; i<parent->children_count; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = NULL;
            parent->children_count--;
            child->parent = NULL;
            return;
        }
    }
    logDebug("Child not found.");
}

void Widget_Draw(Widget *self, Canvas *canvas) {
    if (!self) {
        logDebug("Cannot draw NULL Widget");
        return;
    }

    canvas->current_style = self->style;
    Canvas_Fill(canvas, self->style);

    if (self->ops && self->ops->draw) {
       self->ops->draw(self, canvas);
    }

    for (int i=0; i<self->children_count; i++) {
        Widget *child = self->children[i];
        if (!child) {
            continue;
        }
        
        Canvas child_canvas;
        Canvas_Init(&child_canvas, child->width, child->height);
        Widget_Draw(self->children[i], &child_canvas);
        Canvas_ClipTo(&child_canvas, canvas, child->x, child->y);
        Canvas_Deinit(&child_canvas);
    }
}

void Widget_onParentResize(Widget *self, int new_parent_width, int new_parent_height) {
    if (!self) {
        logError("Invalid widget.");
        return;
    }

    if (self->ops && self->ops->on_resize) {
        self->ops->on_resize(self, new_parent_width, new_parent_height);
    }

    // Propagate the resize event to children, passing the NEW size of THIS widget.
    for (int i=0; i<self->children_count; i++) {
        Widget *child = self->children[i];
        if (!child) {
            continue;
        }
        Widget_onParentResize(child, self->width, self->height);
    } 
}

void Widget_HandleInput(Widget *self, EscapeSequence key, UTF8Char ch) {
    if (!self) {
        logError("Invalid widget.");
        return;
    }
    if (self->ops && self->ops->on_input) {
        self->ops->on_input(self, key, ch);
    }
    for (int i=0; i<self->children_count; i++) {
        Widget *child = self->children[i];
        if (!child) {
            continue;
        }
        Widget_HandleInput(child, key, ch);
    }
}
