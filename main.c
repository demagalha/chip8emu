#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>
#include "chip8.h"
#include "screen.h"

int main(int argc, char** argv) {
    Chip8 chip8;

    // Init
    chip8_init(&chip8);

    srand(time(NULL)); // seed the RNG for the instruction that will use rand

    if (argc != 2) {
        printf("Can only accept rom file as an argument\n");
        exit(EXIT_FAILURE);
    }

    int rom_size = load_rom(&chip8, argv[1]);
    if (rom_size == -1) {
        exit(EXIT_FAILURE);
    }

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP-8 Emulator");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        process_input(&chip8);

        // Run more CPU cycles to help with responsiviness of input
        for (int i = 0; i < 10; i++) {
            chip8_emu_cycle(&chip8);
        }

        // Update timers at 60Hz:
        update_timers(&chip8);

        // Draw if draw_flag
        draw_graphics(&chip8);
    }
    CloseWindow();

    return 0;

}