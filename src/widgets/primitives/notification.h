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
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "display/widget.h"
#include "io/timer.h"
#include "common/utf8string.h"

typedef enum {
    NOTIFICATION_NORMAL,
    NOTIFICATION_SUCCESS,
    NOTIFICATION_ERROR,
    NOTIFICATION_WARNING
} NotificationType;

typedef struct _Notification {
    Widget base;
    UTF8String text;
    NotificationType type;
    uint8_t timer;
} Notification;

#define AS_NOTIFICATION(w) ((Notification*)(w))


void Notification_Init(Notification *self, Widget *parent);
Notification *Notification_Create(Widget *parent);

void Notification_Notify(Notification *self, const char *msg, NotificationType type);

#endif