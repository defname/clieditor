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
#ifndef APP_H
#define APP_H

#include "display/widget.h"
#include "widgets/primitives/notification.h"

typedef struct {
    Widget base;
    Notification *notification;
} App;

extern App app;

void App_Init(int width, int height);
void App_Deinit();

bool App_HandleInput(InputEvent input);
void App_Draw(Canvas *canvas);

void App_onParentResize(int new_parent_width, int new_parent_height);

#endif