#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

static const unsigned char chip8_fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static const unsigned char key_map[NUM_KEYS] = {
    KEY_ZERO,
    KEY_ONE, // 1
    KEY_TWO, // 2
    KEY_THREE, // 3
    KEY_Q, // 4
    KEY_W, // 5
    KEY_E, // 6
    KEY_A, // 7
    KEY_S, // 8
    KEY_D, // 9
    KEY_Z, // A
    KEY_C, // B
    KEY_FOUR, // C
    KEY_R, // D
    KEY_F, // E
    KEY_V, // F
};



void chip8_emu_cycle(Chip8* chip8) {

    // Fetch opcode
    unsigned short opcode = chip8->mem[chip8->pc] << 8 | chip8->mem[chip8->pc + 1]; // OUR OPCODES ARE TWO BYTES LONGS, THEREFORE WE MERGE FROM MEMORY WITH A SHIFT OF 1 BYTE + BITWISE OR

    // Now let's decode the OPCODE, THERE ARE 35 OPCODES

    // to help a little
    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short y = (opcode & 0x00F0) >> 4;
    unsigned short NNN = opcode & 0x0FFF; // this bitwise & WILL RETURN NNN (see 0XA000)
    unsigned short NN = opcode & 0x00FF;
    unsigned short N = opcode & 0X000F; // Height for the DXYN draw opcode
    
    switch (opcode & 0xF000) // GET THE FIRST NIBBLE
    {
        case 0xA000: // ANNN: Sets I to the address NNN
            chip8->I = NNN;
            chip8->pc += 2;
            break;

        case 0xB000: // BNNN: Jumps to the address NNN plus V0
            chip8->pc = chip8->V[0x0] + NNN; // 0 is 0 in all bases but whatever
            break;

        case 0xC000: { // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
            unsigned char rand_num = rand() % 256;
            chip8->V[x] = rand_num & NN;
            chip8->pc += 2;
            break;
        }

        // begin from implementing EX9E
        case 0xE000:
            switch(opcode & 0x00FF) {
                case 0x009E: // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is pressed
                    if (chip8->key[chip8->V[x]]) {
                        chip8->pc += 4;
                    } else {
                        chip8->pc += 2;
                    }
                    break;

                case 0x00A1: // Skips the next instruction if the key stored in VX(only consider the lowest nibble) is not pressed
                    if (!chip8->key[chip8->V[x]]) {
                        chip8->pc += 4;
                    } else {
                        chip8->pc += 2;
                    }
                    break;

                default:
                    printf("Not yet implemented or unknown opcode 0x%X\n", opcode);
                    exit(EXIT_FAILURE);
            }
            break;

        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007: // FX07: Sets VX to the value of the delay timer
                    chip8->V[x] = chip8->delay_timer;
                    chip8->pc += 2;
                    break;
                
                case 0x000A: { // FX0A: A key press is awaited, and then stored in VX
                    chip8->key_pressed = 0;
                    for (int i = 0; i < NUM_KEYS; i++) {
                        if (chip8->key[i]) {
                            chip8->V[x] = i;
                            chip8->key_pressed = 1;
                            break;
                        }
                    }
                    if (!chip8->key_pressed) {
                        return;
                    }

                    chip8->pc += 2;
                    break;
                }
                
                case 0x0015: // FX15: Sets the delay timer to VX
                    chip8->delay_timer = chip8->V[x];
                    chip8->pc += 2;
                    break;
                    
                case 0x0018: // FX18: Sets the sound timer to VX
                    chip8->sound_timer = chip8->V[x];
                    chip8->pc += 2;
                    break;

                case 0x001E: // FX1E: Adds VX to I. VF is not affected
                    chip8->I += chip8->V[x];
                    chip8->pc += 2;
                    break;

                case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX(only consider the lowest nibble)
                    //Characters 0-F (in hexadecimal) are represented by a 4x5 font
                    chip8->I = 0x050 + 5 * chip8->V[x];
                    chip8->pc += 2;
                    break;

                case 0x0033: { // FX33: Stores the binary-coded decimal representation of VX, 
                    // with the hundreds digit in memory at location in I, 
                    // the tens digit at location I+1, and the ones digit at location I+2
                    unsigned char value = chip8->V[x];
                    // let's do it backwards
                    chip8->mem[chip8->I + 2] = value % 10;
                    value /= 10;
                    
                    chip8->mem[chip8->I + 1] = value % 10;
                    value /= 10;

                    chip8->mem[chip8->I] = value % 10;

                    chip8->pc += 2;
                    break;
                }

                case 0x0055: // FX55: Stores from V0 to VX (including VX) in memory,
                    //starting at address I. The offset from I is increased by 1 for each value written,
                    //but I itself is left unmodified
                    for (int i = 0; i <= x; i++) {
                        chip8->mem[chip8->I + i] = chip8->V[i];
                    }
                    chip8->pc += 2;
                    break;

                case 0x0065: // FX65: Fills from V0 to VX (including VX) with values from memory, 
                    // starting at address I. The offset from I is increased by 1 for each value read, 
                    // but I itself is left unmodified
                    for (int i = 0; i <= x; i++) {
                        chip8->V[i] = chip8->mem[chip8->I + i];
                    }
                    chip8->pc += 2;
                    break;

                default:
                    printf("Not yet implemented or unknown opcode 0x%X\n", opcode);
                    exit(EXIT_FAILURE);
            }
            break;
        case 0x1000: // 1NNN: Jumps to address NNN
            chip8->pc = NNN;
            break;
        
        case 0x2000: // 2NNN: Calls subroutine at NNN
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = NNN;
            break;
        
        case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
            if (chip8->V[x] == NN) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;
        
        case 0x4000: // 4XNN: Skips the next instruction if VX does not equal NN
            if (chip8->V[x] != NN) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;

        case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
            if (chip8->V[x] == chip8->V[y]) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;

        case 0x6000: // 6XNN: Sets VX to NN=
            chip8->V[x] = NN;
            chip8->pc += 2;
            break;

        case 0x7000: // 7XNN: Adds NN to VX (carry flag is not changed)
            chip8->V[x] += NN;
            chip8->pc += 2;
            break;

        case 0x8000: // ONE OF THE PROBLEMS WITH THESE OPCODE ARE IF VX == VY
            switch(opcode & 0x000F) {
                case 0x0000: // 8XY0: Sets VX to the value of VY
                    chip8->V[x] = chip8->V[y];
                    chip8->pc +=2;
                    break;

                case 0x0001: // 8XY1: Sets VX to VX or VY. (bitwise OR operation)
                    chip8->V[x] = (chip8->V[x] | chip8->V[y]);
                    chip8->pc += 2;
                    break;

                case 0x0002: // 8XY2: Sets VX to VX and VY. (bitwise AND operation)
                    chip8->V[x] = (chip8->V[x] & chip8->V[y]);
                    chip8->pc += 2;
                    break;

                case 0x0003: // 8XY3: Sets VX to VX xor VY
                    chip8->V[x] = (chip8->V[x] ^ chip8->V[y]);
                    chip8->pc += 2;
                    break;
                
                case 0x0004: { // 8XY4: Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not
                    // the register are 8 Bits, so 2^8 = 256
                    unsigned short sum = chip8->V[x] + chip8->V[y];
                    chip8->V[x] = sum & 0xFF;

                    if (sum > 255) {
                        chip8->V[0xF] = 1;
                    } else {
                        chip8->V[0xF] = 0;
                    }

                    chip8->pc += 2;
                    break;
                }

                case 0x0005: { // 8XY5: VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not)
                    unsigned char not_underflow = chip8->V[x] >= chip8->V[y] ? 1 : 0;
                    chip8->V[x] -= chip8->V[y];
                    chip8->V[0xF] = not_underflow;
                    
                    chip8->pc += 2;
                    break;
                }

                case 0x0006: // 8XY6: Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF
                    unsigned char LSB = chip8->V[x] & 0x01; // Store LSB before shift
                    chip8->V[x] >>= 1;
                    chip8->V[0xF] = LSB;

                    chip8->pc += 2;
                    break;

                case 0x0007: { // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX)
                    unsigned char not_underflow = chip8->V[y] >= chip8->V[x] ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->V[0xF] = not_underflow;

                    chip8->pc += 2;
                    break;
                }
                
                case 0x000E: // 8XYE: Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset
                    unsigned char MSB = (chip8->V[x] & 0x80) >> 7; // Get MSB of V[x] prior to shift
                    chip8->V[x] <<= 1;
                    if (MSB == 1) {
                        chip8->V[0xF] = 1;
                    } else {
                        chip8->V[0xF] = 0;
                    }

                    chip8->pc += 2;
                    break;

                default:
                    printf("Not yet implemented or unknown opcode 0x%X\n", opcode);
                    exit(EXIT_FAILURE);
            }
            break;

        case 0X9000: // 9XY0: Skips the next instruction if VX does not equal VY
            if (chip8->V[x] != chip8->V[y]) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;
        

        case 0xD000: { // DXYN: Draw a sprite at (Vx, Vy) with width 8 and height N
            unsigned char vx = chip8->V[x];
            unsigned char vy = chip8->V[y];
            int height = N;
            
            chip8->V[0xF] = 0; // Resetting colision flag
            for(int row = 0; row < height; row++) {
                unsigned char sprite = chip8->mem[chip8->I + row];
                for(int col = 0; col < 8; col++) {
                    unsigned char pixel = (sprite >> (7-col)) & 1;
                    int index = ((vy + row) % SCREEN_HEIGHT) * SCREEN_WIDTH + ((vx + col) % SCREEN_WIDTH);

                    if (pixel && chip8->screen[index] == 1) {
                        chip8->V[0xF] = 1;
                    }
                    chip8->screen[index] ^= pixel;
                }
            }
            
            chip8->draw_flag = 1;
            chip8->pc += 2;
            break;
        }
        
        case 0x0000:
            switch(opcode & 0x00FF) {
                case 0x00E0: // Clear screen
                    clear_screen(chip8);
                    chip8->draw_flag = 1;
                    chip8->pc += 2;
                    break;
                
                case 0x00EE:
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    chip8->pc += 2;
                    break;

                default:
                    printf("Not yet implemented or unknown opcode 0x%X\n", opcode);
                    exit(EXIT_FAILURE);

            }
            break;
        default:
            printf("Not yet implemented or unknown opcode 0x%X\n", opcode);
            exit(EXIT_FAILURE);
    }
}

void chip8_init(Chip8* chip8) {
    chip8->pc = PC_START_ADDR;
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    chip8->key_pressed = 0;

    // Load fontset into memory starting at 0x050
    for(int i = 0; i < FONTSET_SIZE; ++i) {
        chip8->mem[0x050 + i] = chip8_fontset[i]; //load from 0X050 onwards
    }

    //clear stack, registers and set keys to NOT pressed
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->V, 0, NUM_V_REGISTERS);
    memset(chip8->key, 0, NUM_KEYS);
    memset(chip8->screen, 0, sizeof(chip8->screen));

}

int load_rom(Chip8* chip8, char* rom_name) {
    long rom_size;
    unsigned char* rom_buffer;

    FILE *rom = fopen(rom_name, "rb");

    if (rom == NULL) {
        perror("Failed to open ROM");
        return -1;
    }

    // Move file position to end
    fseek(rom, 0, SEEK_END);

    // Get rom size
    rom_size = ftell(rom);
    rewind(rom);

    if (rom_size > MEMORY_SIZE - PC_START_ADDR) { // (end address - start address of rom in memory)
        perror("Room too big");
        fclose(rom);
        return -1;
    }
    

    rom_buffer = malloc(sizeof(unsigned char) * rom_size);
    if (rom_buffer == NULL) {
        perror("No memory");
        fclose(rom);
        return -1;
    }

    fread(rom_buffer, sizeof(unsigned char), rom_size, rom);

    // Now let's load into memory
    for(int i = 0; i < rom_size; ++i)
        chip8->mem[i + PC_START_ADDR] = rom_buffer[i]; //chip8->mem[i + 0x200]

    fclose(rom);
    free(rom_buffer);


    printf("%d\n", rom_size);
    return rom_size;
}

void clear_screen(Chip8* chip8) {
        memset(chip8->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void process_input(Chip8* chip8) {
    for (int i = 0; i < NUM_KEYS; i++) {
        if (IsKeyDown(key_map[i])) {
            chip8->key[i] = 1;
        } else {
            chip8->key[i] = 0;
        }
    }
}

void update_timers(Chip8* chip8) {
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    if (chip8->sound_timer > 0) {
        chip8->sound_timer--;
        if (chip8->sound_timer == 0) {
            // TODO: play beep sound or stop beep here
            printf("Beep\n");
        }
    }
}
