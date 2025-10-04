#ifndef CALLBACK_H
#define CALLBACK_H

typedef void (*CallbackFn)(void *caller, void *data);

typedef struct _Callback {
    CallbackFn fn;
    void *data;
} Callback;

Callback Callback_New(CallbackFn fn, void *data);
void Callback_Call(Callback *cb, void *caller);

#endif