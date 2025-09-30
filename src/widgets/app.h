#ifndef APP_H
#define APP_H

#include "display/widget.h"

typedef struct {
    int dummy;
} AppData;

Widget *App_Create(int width, int height);


#endif