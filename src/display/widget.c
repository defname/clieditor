#include "widget.h"

#include <stdlib.h>
#include "common/logging.h"

void Widget_Init(Widget *widget, Widget *parent, WidgetOps *ops) {
    widget->ops = ops;

    widget->x = 0;
    widget->y = 0;
    widget->width = 0;
    widget->height = 0;

    widget->z_index = 0;

    widget->has_focus = false;
    widget->return_focus_to = NULL;
    widget->visible = true;


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
        self->children[i] = NULL;
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

void Widget_SortTreeByZIndex(Widget *root) {
    if (!root) {
        return;
    }
    qsort(root->children, root->children_count, sizeof(Widget *), compare_children_z_index);
    for (int i=0; i<root->children_count; i++) {
        Widget_SortTreeByZIndex(root->children[i]);
    }
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

Widget *Widget_LCP(const Widget *a, const Widget *b) {
    if (!a || !b) return NULL;

    // 1. find depth of both widgets
    int depth_a = 0, depth_b = 0;
    const Widget *tmp;

    for (tmp = a; tmp; tmp = tmp->parent) depth_a++;
    for (tmp = b; tmp; tmp = tmp->parent) depth_b++;

    // 2. ascent the deeper widget until both are on the same height
    while (depth_a > depth_b) { a = a->parent; depth_a--; }
    while (depth_b > depth_a) { b = b->parent; depth_b--; }

    // 3. ascent both until a common ancestor is found
    while (a && b && a != b) {
        a = a->parent;
        b = b->parent;
    }

    // 4. return LCP or NULL if none was found
    return (Widget *)a;
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
        if (!child || !child->visible) {
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

bool Widget_HandleInput(Widget *self, EscapeSequence key, UTF8Char ch) {
    if (!self || !self->has_focus) {
        logError("Invalid widget.");
        return false;
    }
    // check if the widget handles input
    if (self->ops && self->ops->on_input) {
        if (self->ops->on_input(self, key, ch)) {
            return true;
        }
    }
    // if self does not handle the input (not on_input() or on_input() == false)
    // bubble up
    Widget *child_with_focus = Widget_ChildHasFocus(self);
    if (child_with_focus) {
        return Widget_HandleInput(child_with_focus, key, ch);
    }
    return false;
}

Widget *Widget_ChildHasFocus(Widget *self) {
    for (int i=0; i<self->children_count; i++) {
        Widget *child = self->children[i];
        if (!child) {
            continue;
        }
        if (child->has_focus) {
            return child;
        }
    }
    return NULL;
}

static void focus_recursively(Widget *self) {
    if (!self || self->has_focus) {
        return;
    }

    focus_recursively(self->parent);
    self->has_focus = true;
    if (self->ops && self->ops->on_focus) {
        self->ops->on_focus(self);
    }
}

void Widget_Focus(Widget *self) {
    if (!self || self->has_focus) {
        return;
    }

    if (!self->visible) {
        Widget_Show(self);
    }
    /*
            F                       F
           / \                     / \
          F   X <- focus   =>     X   F
          |                       |
          F                       X
    */
    // find the first predecessor that is in focus
    Widget *predecessor_in_focus = self->parent;
    while (predecessor_in_focus && !predecessor_in_focus->has_focus) {
        predecessor_in_focus = predecessor_in_focus->parent;
    }
    if (predecessor_in_focus) {
        // if it has a child in focus blur it
        Widget *predecessor_child = Widget_ChildHasFocus(predecessor_in_focus);
        if (predecessor_child) {
            Widget_Blur(predecessor_child);
        }
    }

    focus_recursively(self);
}

void Widget_FocusAndReturn(Widget *widget, Widget *caller) {
    Widget *focus_leaf = Widget_GetFocusLeaf(caller);
    Widget_Focus(widget);
    widget->return_focus_to = focus_leaf;
}

void Widget_Blur(Widget *self) {
    if (!self || !self->has_focus) {
        return;
    }
    /*
            F         F
           / \       / \
   blur-> F   X  => X   X
          |         |
          F         X
    */
    // if a child has focus blur it first
    Widget *child_in_focus = Widget_ChildHasFocus(self);
    if (child_in_focus) {
        Widget_Blur(child_in_focus);
    }
    // blur self
    self->has_focus = false;
    if (self->ops && self->ops->on_blur) {
        self->ops->on_blur(self);
    }
    if (self->return_focus_to) {
        Widget *new_focus =  self->return_focus_to;
        self->return_focus_to = NULL;
        Widget_Focus(new_focus);
    }
}

Widget *Widget_GetFocusLeaf(Widget *root) {
    if (!root || !root->has_focus) {
        return NULL;
    }
    while (1) {
        Widget *child_with_focus = Widget_ChildHasFocus(root);
        if (!child_with_focus) {
            break;
        }
        root = child_with_focus;
    }
    return root;
}

void Widget_Show(Widget *self) {
    if (!self || self->visible) {
        return;
    }
    self->visible = true;
    Widget_Show(self->parent);
}

void Widget_Hide(Widget *self) {
    if (!self || !self->visible) {
        return;
    }
    if (self->has_focus) {
        Widget_Blur(self);
    }
    self->visible = false;
    for (int i=0; i<self->children_count; i++) {
        Widget *child = self->children[i];
        if (!child) {
            continue;
        }
        Widget_Hide(child);
    }
}
