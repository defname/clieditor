#ifndef WIDGET_H
#define WIDGET_H

#include <stdbool.h>

#include "display/canvas.h"
#include "io/input.h"

#define WIDGET_INITIAL_CAPACITY 4
#define WIDGET_GROWTH_FACTOR 2


// Forward-declaration for the Widget struct
struct _Widget;

// "Virtual methods" for a Widget.
typedef struct {
    // Draws the widget onto a target canvas.
    void (*draw)(const struct _Widget *self, Canvas *target);
    // Handles input. Returns true if the input was consumed.
    bool (*on_input)(struct _Widget *self, EscapeSequence key, UTF8Char ch);
    // Handles resize events
    void (*on_resize)(struct _Widget *self, int new_parent_width, int new_parent_height);
    // Called (by App_SetFocus()) when the widget gains focus.
    void (*on_focus)(struct _Widget *self);
    // Called (by App_ClearFocus()) when the widget looses focus.
    void (*on_blur)(struct _Widget *self);
    // Frees all resources associated with the widget including widget->data, *but not* the widget itself
    void (*destroy)(struct _Widget *self);
} WidgetOps;

// A Widget is an active UI component with logic and state.
typedef struct _Widget {
    // Layout properties (relative to the parent)
    int x, y;
    int width, height;

    int z_index;

    bool has_focus;
    struct _Widget *return_focus_to;
    bool visible;

    // Pointers to the widget's "methods"
    WidgetOps *ops;

    Style style;

    // Tree structure
    struct _Widget *parent;
    struct _Widget **children;
    int children_count;
    int children_capacity;
} Widget;

#define AS_WIDGET(w) ((Widget*)(w))


void Widget_Init(Widget *w, Widget *parent, WidgetOps *ops);
void Widget_Deinit(Widget *w);

Widget *Widget_Create(Widget *parent, WidgetOps *ops);
void Widget_Destroy(Widget *self);

void Widget_AddChild(Widget *self, Widget *child);
void Widget_RemoveChild(Widget *self, Widget *child);
Widget *Widget_LCP(const Widget *a, const Widget *b);

void Widget_SetZIndex(Widget *self, int z_index);

void Widget_Draw(Widget *self, Canvas *canvas);
void Widget_onParentResize(Widget *self, int new_parent_width, int new_parent_height);

bool Widget_HandleInput(Widget *self, EscapeSequence key, UTF8Char ch);

Widget *Widget_ChildHasFocus(Widget *self);
void Widget_Focus(Widget *widget);
void Widget_FocusAndReturn(Widget *widget, Widget *caller);
void Widget_Blur(Widget *widget);
Widget *Widget_GetFocusLeaf(Widget *root);  //< return the most outer Widget that has focus (root needs to be any Widget in the focus chain). you must be careful not to destroy a widget before it was returned to, to prevent dangling pointers

void Widget_Show(Widget *widget);
void Widget_Hide(Widget *widget);


#endif