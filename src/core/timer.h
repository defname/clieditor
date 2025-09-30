#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef enum {
    TIMER_INACTIVE,
    TIMER_RUNNING,
    TIMER_PAUSED,
    TIMER_FINISHED
} TimerState;

typedef void (*TimerCallback)(void *user_data);

typedef uint32_t Miliseconds;

typedef struct _Timer {
    uint8_t id;
    TimerState state;
    Miliseconds remaining;
    Miliseconds duration;
    TimerCallback callback;
    void *user_data;
} Timer;


void Timer_Init();
void Timer_Deinit();
void Timer_Update();

uint8_t Timer_Start(Miliseconds time, TimerCallback callback, void *user_data);
void Timer_Stop(uint8_t id);
void Timer_Pause(uint8_t id);
void Timer_Resume(uint8_t id);


#endif