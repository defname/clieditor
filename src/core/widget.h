#ifndef WIDGET_H
#define WIDGET_H

#include <stdbool.h>

#include "canvas.h"
#include "input.h"

#define WIDGET_INITIAL_CAPACITY 4
#define WIDGET_GROWTH_FACTOR 2


// Forward-declaration for the Widget struct
struct _Widget;

// "Virtual methods" for a Widget.
typedef struct {
    // Draws the widget onto a target canvas.
    void (*draw)(struct _Widget *self, Canvas *target);
    // Handles input. Returns true if the input was consumed.
    bool (*handle_input)(struct _Widget *self, EscapeSequence key, UTF8Char ch);
    // Frees all resources associated with the widget.
    void (*destroy)(struct _Widget *self);
} WidgetOps;

// A Widget is an active UI component with logic and state.
typedef struct _Widget {
    // Layout properties (relative to the parent)
    int x, y;
    int width, height;

    // Widget-specific data (e.g., a pointer to a struct with text content)
    void *data;
    // Pointers to the widget's "methods"
    WidgetOps *ops;

    // Tree structure
    struct _Widget *parent;
    struct _Widget **children;
    int children_count;
    int children_capacity;
} Widget;

void Widget_Init(Widget *w, Widget *parent, WidgetOps *ops);
void Widget_Deinit(Widget *w);

Widget *Widget_Create(Widget *parent, WidgetOps *ops);
void Widget_Destroy(Widget *self);

void Widget_AddChild(Widget *self, Widget *child);
void Widget_RemoveChild(Widget *self, Widget *child);

#endif