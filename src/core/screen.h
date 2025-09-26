/**
 * screen.h
 * Abstracts the screen of the terminal.
 * Moving the cursor writing output to the screen, colorize, etc. can be done with this module.
 * 
 * The Screen consists of Cells which hold an UTFChar and a Style.
 */
#ifndef SCREEN_H
#define SCREEN_H

#include <stddef.h>
#include <stdint.h>
#include "utf8.h"

#define STYLE_NONE      0
#define STYLE_BOLD      1
#define STYLE_UNDERLINE 2

typedef struct {
    uint8_t fg; // foreground color (ANSI-Code 0-255)
    uint8_t bg; // background color (ANSI-Code 0-255)
    uint16_t attributes; // Bit-Flags für fett, unterstrichen etc.
} Style;

typedef struct _Cell {
    UTF8Char ch;
    Style style;
    bool changed;  // marks if the Cell has changed since the last rendering
} Cell;

typedef struct _Screen {
    Cell *buffer;
    size_t size;
    size_t capacity;
    int cursor_col;
    int cursor_row;
    Style current_style;
} Screen;

extern Screen screen;

void Screen_Init();  // call after Terminal_Init()!!!
void Screen_Finish();
void Screeen_Update();  // Reallocates memory for the buffer. Need to be called AFTER Terminal_Update()


// The following functions are buffered. All changes are drawn by Screen_Draw()
void Screen_Clear();
void Screen_MoveCursor(int col, int row);
void Screen_HideCursor();
void Screen_ShowCursor();
void Screen_PutChar(UTF8Char c);  // put c to the current position and move the cursor
void Screen_Write(const UTF8Char *s, size_t n);  // write s to the current position and move the cursor

void Screen_Draw();

#endif