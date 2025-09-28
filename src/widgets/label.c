#include "label.h"
#include <stdlib.h>
#include <string.h>
#include "../core/canvas.h"
#include "../utils/logging.h"

// 1. Spezifische Daten für das Label

// 2. Spezifische Implementierungen der Operationen

// Die "draw"-Methode für ein Label
void label_draw(Widget *self, Canvas *canvas) {
    if (!self || !self->data) return;
    LabelData *data = (LabelData*)self->data;

    // Schreibe den Text auf den Canvas
    Canvas_MoveCursor(canvas, self->x, self->y);
    Canvas_Write(canvas, data->text.chars, data->text.length);
}

// Die "destroy"-Methode für ein Label (gibt den Text frei)
void label_destroy(Widget *self) {
    if (self && self->data) {
        LabelData *data = (LabelData*)self->data;
        UTF8String_Deinit(&data->text);
        free(data);
        self->data = NULL;
    }
    // Rufe die Basis-Destroy-Funktion auf
    Widget_Destroy(self);
}

// 3. Die "vtable" für das Label-Widget
static WidgetOps label_ops = {
    .draw = label_draw,
    .destroy = label_destroy,
    // .handle_input = NULL, // Ein Label reagiert nicht auf Input
};

// 4. Der "Konstruktor"
Widget* Label_Create(Widget *parent, const char* text) {
    Widget *widget = Widget_Create(parent, &label_ops);
    LabelData *data = malloc(sizeof(LabelData));
    UTF8String_FromStr(&data->text, text, strlen(text));
    widget->data = data;
    return widget;
}