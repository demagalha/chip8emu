#ifndef SCREEN_H
#define SCREEN_H

#include "chip8.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10
#define WINDOW_WIDTH (SCREEN_WIDTH * SCALE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * SCALE)

void draw_graphics(Chip8* chip8);

#endif /*SCREEN_H*/