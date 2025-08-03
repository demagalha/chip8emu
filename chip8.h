#ifndef CHIP8_H
#define CHIP8_H

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define NUM_V_REGISTERS 16
#define NUM_KEYS 16
#define FONTSET_SIZE 80
#define PC_START_ADDR 0x200

typedef struct {
    unsigned char mem[MEMORY_SIZE]; // 4k memory
    unsigned char V[NUM_V_REGISTERS]; // 15 8 Bit Registers, V0... VE
    unsigned short I; // Index register
    unsigned short pc; // The program counter

    unsigned char screen[SCREEN_WIDTH * SCREEN_HEIGHT]; // Our screen, to be XOR'D when we want to draw

    unsigned char delay_timer, sound_timer; // Two timer registers

    unsigned short stack[STACK_SIZE];
    unsigned short sp; // Stack Pointer

    unsigned char key[NUM_KEYS]; // HEX KEYPAD

    unsigned char chip8_fontset[FONTSET_SIZE];

    int draw_flag;

    // need this
    int key_pressed;

} Chip8;

void chip8_emu_cycle(Chip8* chip8);
void chip8_init(Chip8* chip8);
int load_rom(Chip8* chip8, char* rom_name);
void clear_screen(Chip8* chip8);
void process_input(Chip8* chip8);
void update_timers(Chip8* chip8);

#endif /*CHIP8_H*/