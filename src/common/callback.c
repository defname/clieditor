#include "callback.h"

Callback Callback_New(CallbackFn fn, void *data) {
    return (Callback){ .fn = fn, .data = data };
}

void Callback_Call(Callback *callback, void *caller) {
    if (callback->fn) {
        callback->fn(caller, callback->data);
    }
}