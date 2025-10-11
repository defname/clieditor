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
/**
 * timer.h
 * Provides a timer system where you can start, stop pause and resume timers.
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define MAX_TIMER 32
#define NO_TIMER MAX_TIMER

typedef enum {
    TIMER_INACTIVE,
    TIMER_RUNNING,
    TIMER_PAUSED,
    TIMER_FINISHED
} TimerState;

// callback function takes the id of the timer calling it and additional data 
typedef void (*TimerCallback)(uint8_t timer_id, void *user_data);
typedef uint32_t Milliseconds;

void Timer_Init();          // call this when initializing the program
void Timer_Deinit();        // not used at the moment
Milliseconds Timer_Update(); // call this in every iteration of the main loop. return time in ms since last call

// return id of the created timer
uint8_t Timer_Start(Milliseconds time,       // time in ms the timer should run
                    TimerCallback callback, // function that will be called at timeout
                    void *user_data);       // additional data to pass to the callback function
void Timer_Restart(uint8_t id);
void Timer_Stop(uint8_t id);
void Timer_Pause(uint8_t id);
void Timer_Resume(uint8_t id);

#endif