#include "timer.h"

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

typedef struct _Timer {
    TimerState state;
    Milliseconds remaining;
    Milliseconds duration;
    TimerCallback callback;
    void *user_data;
} Timer;

static struct timeval last_update_time;
static Timer timers[MAX_TIMER];

static void init_timer(Timer *timer) {
    timer->state = TIMER_INACTIVE;
    timer->remaining = 0;
    timer->duration = 0;
    timer->callback = NULL;
    timer->user_data = NULL;
}

static void update_timer(Timer *timer, uint8_t timer_id, Milliseconds dt) {
    if (timer->state != TIMER_RUNNING) {
        if (timer->state == TIMER_FINISHED) {
            // Reset the timer in the next cycle after it has finished.
            // This gives the callback a chance to restart it.
            init_timer(timer);
        }
        return;
    }

    if (dt >= timer->remaining) {
        timer->remaining = 0;
        timer->state = TIMER_FINISHED;
        if (timer->callback) {
            timer->callback(timer_id, timer->user_data);
        }
    } else {
        timer->remaining -= dt;
    }
}

void Timer_Init() {
    gettimeofday(&last_update_time, NULL);

    for (int i=0; i<MAX_TIMER; i++) {
        init_timer(&timers[i]);
    }
}

void Timer_Deinit() {
    // No special deinitialization needed for the timers array itself.
    // The resources are managed statically.
}

Milliseconds Timer_Update() {
    struct timeval now;
    gettimeofday(&now, NULL);

    uint64_t now_ms = (uint64_t)now.tv_sec * 1000 + now.tv_usec / 1000;
    uint64_t last_ms = (uint64_t)last_update_time.tv_sec * 1000 + last_update_time.tv_usec / 1000;
    Milliseconds dt = now_ms - last_ms;

    last_update_time = now;

    for (int i=0; i<MAX_TIMER; i++) {
        update_timer(&timers[i], i, dt);
    }

    return dt;
}

uint8_t Timer_Start(Milliseconds time, TimerCallback callback, void *user_data) {
    // find free slot
    for (int i = 0; i < MAX_TIMER; i++) {
        if (timers[i].state == TIMER_INACTIVE) {
            Timer *timer = &timers[i];
            timer->state = TIMER_RUNNING;
            timer->remaining = time;
            timer->duration = time;
            timer->user_data = user_data;
            timer->callback = callback;
            return i;
        }
    }
    return NO_TIMER;
}

void Timer_Restart(uint8_t id) {
    if (id >= MAX_TIMER) {
        return;
    }
    Timer *timer = &timers[id];
    if (timer->state == TIMER_INACTIVE) {
        return;
    }
    timer->state = TIMER_RUNNING;
    timer->remaining = timer->duration;
}

void Timer_Stop(uint8_t id) {
    if (id >= MAX_TIMER) {
        return;
    }
    init_timer(&timers[id]);
}

void Timer_Pause(uint8_t id) {
    if (id >= MAX_TIMER) {
        return;
    }
    Timer *timer = &timers[id];
    if (timer->state != TIMER_RUNNING) {
        return;
    }
    timer->state = TIMER_PAUSED;
}

void Timer_Resume(uint8_t id) {
    if (id >= MAX_TIMER) {
        return;
    }
    Timer *timer = &timers[id];
    if (timer->state != TIMER_PAUSED) {
        return;
    }
    timer->state = TIMER_RUNNING;
}
