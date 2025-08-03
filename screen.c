#include "screen.h"
#include <raylib.h>

void draw_graphics(Chip8* chip8) {
    BeginDrawing();

    if (chip8->draw_flag) {
        ClearBackground(BLACK);

        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                int index = y * SCREEN_WIDTH + x;
                if (chip8->screen[index]) {
                    DrawRectangle(x * SCALE, y * SCALE, SCALE, SCALE, WHITE);
                }
            }
        }
        chip8->draw_flag = 0;
    }
    EndDrawing();
}