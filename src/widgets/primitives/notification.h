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