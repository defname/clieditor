/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "label.h"
#include <stdlib.h>
#include <string.h>
#include "display/canvas.h"
#include "common/logging.h"

// 1. Spezifische Daten für das Label

// 2. Spezifische Implementierungen der Operationen

// Die "draw"-Methode für ein Label
static void label_draw(const Widget *self, Canvas *canvas) {
    // Schreibe den Text auf den Canvas
    Canvas_MoveCursor(canvas, 0, 0);
    Canvas_Write(canvas, &(AS_LABEL(self)->text));
}

// Die "destroy"-Methode für ein Label (gibt den Text frei)
static void label_destroy(Widget *self) {
    String_Deinit(&(AS_LABEL(self))->text);
}

// 3. Die "vtable" für das Label-Widget
static WidgetOps label_ops = {
    .draw = label_draw,
    .destroy = label_destroy,
    // .on_input = NULL, // Ein Label reagiert nicht auf Input
};

// 4. Der "Konstruktor"

void Label_Init(Label *self, Widget *parent, const char *text) {
    Widget_Init(&self->base, parent, &label_ops);
    self->text = String_FromCStr(text, strlen(text));
}

Label *Label_Create(Widget *parent, const char* text) {
    Label *new = malloc(sizeof(Label));
    if (!new) {
        logFatal("Cannot allocate memory for Label.");
    }
    Label_Init(new, parent, text);
    
    return new;
}
