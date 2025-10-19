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
#include "common/config.h"


static void hide_notification(uint8_t timer_id, void *user_data) {
    Notification *noty = (Notification*)user_data;
    Widget_Hide(AS_WIDGET(noty));
    Timer_Stop(timer_id);
    noty->timer = NO_TIMER;
}

static void notification_destroy(Widget *self) {
    Notification *noty = AS_NOTIFICATION(self);
    String_Deinit(&noty->text);
}

static void notification_draw(const Widget *self, Canvas *canvas) {
    Notification *noty = AS_NOTIFICATION(self);
    switch (noty->type) {
        case NOTIFICATION_SUCCESS:
            canvas->current_style = noty->style_success;
            break;
        case NOTIFICATION_ERROR:
            canvas->current_style = noty->style_error;
            break;
        case NOTIFICATION_WARNING:
            canvas->current_style = noty->style_warning;
            break;
        default:
            canvas->current_style = noty->style_normal;
            break;
    }
    Canvas_Write(canvas, &noty->text);
    String spaces = String_Spaces(self->width - String_Length(&noty->text));
    Canvas_Write(canvas, &spaces);
    String_Deinit(&spaces);
}

static void notification_on_resize(Widget *self, int new_parent_width, int new_parent_height) {
    self->width = 32;
    self->height = 1;
    // place widget at the top right
    self->x = (new_parent_width - self->width) / 2;
    self->y = (new_parent_height - self->height) / 2;
}

static void on_notification_changed(Widget *w) {
    Notification *self = AS_NOTIFICATION(w);
    Table *conf = Config_GetModuleConfig("notification");

    self->style_normal.fg = Config_GetColor(conf, "text", self->style_normal.fg);
    self->style_normal.bg = Config_GetColor(conf, "bg", self->style_normal.bg);
    self->style_error.fg = Config_GetColor(conf, "error.text", self->style_error.fg);
    self->style_error.bg = Config_GetColor(conf, "error.bg", self->style_error.bg);
    self->style_warning.fg = Config_GetColor(conf, "warning.text", self->style_warning.fg);
    self->style_warning.bg = Config_GetColor(conf, "warning.bg", self->style_warning.bg);
    self->style_success.fg = Config_GetColor(conf, "success.text", self->style_success.fg);
    self->style_success.bg = Config_GetColor(conf, "success.bg", self->style_success.bg);
}

WidgetOps notification_ops = {
    .draw = notification_draw,
    .destroy = notification_destroy,
    .on_resize = notification_on_resize,
    .on_config_changed = on_notification_changed,
    .on_input = NULL,
};


void Notification_Init(Notification *self, Widget *parent) {
    Widget_Init(&self->base, parent, &notification_ops);
    Widget_SetZIndex(AS_WIDGET(self), 100 + parent->z_index);
    self->base.visible = false;
    self->timer = NO_TIMER;
    self->type = NOTIFICATION_NORMAL;
    String_Init(&self->text);

    self->style_normal = self->base.style;
    self->style_normal.bg = Color_GetCodeById(COLOR_PRIMARY_BG);
    self->style_error = self->base.style;
    self->style_error.bg = Color_GetCodeById(COLOR_ERROR);
    self->style_warning = self->base.style;
    self->style_warning.bg = Color_GetCodeById(COLOR_WARNING);
    self->style_success = self->base.style;
    self->style_success.bg = Color_GetCodeById(COLOR_SUCCESS);
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
    String_Deinit(&self->text);
    self->text = String_FromCStr(msg, strlen(msg));
    Widget_Show(AS_WIDGET(self));
    self->timer = Timer_Start(2000, hide_notification, self);
}
