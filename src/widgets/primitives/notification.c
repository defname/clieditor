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
#include "notification.h"

#include <string.h>
#include "common/colors.h"


static void hide_notification(uint8_t timer_id, void *user_data) {
    Notification *noty = (Notification*)user_data;
    Widget_Hide(AS_WIDGET(noty));
    Timer_Stop(timer_id);
    noty->timer = NO_TIMER;
}

static void notification_destroy(Widget *self) {
    Notification *noty = AS_NOTIFICATION(self);
    UTF8String_Deinit(&noty->text);
}

static void notification_draw(const Widget *self, Canvas *canvas) {
    Notification *noty = AS_NOTIFICATION(self);
    uint8_t bg;
    switch (noty->type) {
        case NOTIFICATION_SUCCESS:
            bg = Color_GetCodeById(COLOR_SUCCESS);
            break;
        case NOTIFICATION_ERROR:
            bg = Color_GetCodeById(COLOR_ERROR);
            break;
        case NOTIFICATION_WARNING:
            bg = Color_GetCodeById(COLOR_WARNING);
            break;
        default:
            bg = Color_GetCodeById(COLOR_PRIMARY_BG);
            break;
    }
    canvas->current_style.bg = bg;
    canvas->current_style.fg = Color_GetCodeById(COLOR_BG);
    Canvas_Write(canvas, &noty->text);
    UTF8String spaces;
    UTF8String_Init(&spaces);
    UTF8String_Spaces(&spaces, self->width - UTF8String_Length(&noty->text));
    Canvas_Write(canvas, &spaces);
    UTF8String_Deinit(&spaces);
}

static void notification_on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    self->width = 32;
    self->height = 1;
    // place widget at the top right
    self->x = (new_parent_width - self->width) / 2;
    self->y = (new_parent_height - self->height) / 2;
}

WidgetOps notification_ops = {
    .draw = notification_draw,
    .destroy = notification_destroy,
    .on_resize = notification_on_resize,
    .on_input = NULL,
};


void Notification_Init(Notification *self, Widget *parent) {
    Widget_Init(&self->base, parent, &notification_ops);
    Widget_SetZIndex(AS_WIDGET(self), 100 + parent->z_index);
    self->base.visible = false;
    self->timer = NO_TIMER;
    self->type = NOTIFICATION_NORMAL;
    UTF8String_Init(&self->text);
}

Notification *Notification_Create(Widget *parent) {
    Notification *self = malloc(sizeof(Notification));
    Notification_Init(self, parent);
    return self;
}

void Notification_Notify(Notification *self, const char *msg, NotificationType type) {
    if (self->timer != NO_TIMER) {
        Timer_Stop(self->timer);
    }
    self->type = type;
    UTF8String_FromStr(&self->text, msg, strlen(msg));
    Widget_Show(AS_WIDGET(self));
    self->timer = Timer_Start(2000, hide_notification, self);
}
