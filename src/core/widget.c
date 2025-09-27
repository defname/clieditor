#include "widget.h"
#include "utils/logging.h"

void Widget_Init(Widget *widget, Widget *parent, WidgetOps *ops) {
    widget->ops = ops;
    widget->data = NULL;

    widget->x = 0;
    widget->y = 0;
    widget->width = 0;
    widget->height = 0;

    widget->parent = parent;
    widget->children = NULL;
    widget->children_count = 0;
    widget->children_capacity = 0;
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
    Widget_Deinit(self);
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
    for (int i=widget->children_capacity; i<new_capacity; i++) {
        widget->children[i] = NULL;
    }
    widget->children_capacity = new_capacity;
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
            return;
        }
    }
    logDebug("Should not happen.... unreachable code...");
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