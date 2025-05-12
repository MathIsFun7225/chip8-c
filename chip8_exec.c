#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "chip8.h"
#include "chip8_exec.h"

#define bits16(x, size, num) ((uint16_t) (((x) >> (4 * (num))) & ((1 << (size)) - 1)))

static int chip8_stack_push(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t value) {
    (void) cfg;

    if (chip8->sp == chip8->stack_size) {
        uint16_t new_size = (chip8->stack_size == 0 ? 4 : 2 * chip8->stack_size);
        uint16_t *tmp = realloc(chip8->stack, new_size * sizeof(*chip8->stack));
        if (tmp == NULL) {
            fprintf(stderr, "%s: realloc: %s\n", __func__, strerror(errno));
            return -1;
        }
        chip8->stack = tmp;
    }

    chip8->stack[chip8->sp++] = value;
    return 0;
}

static int chip8_stack_pop(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t *value) {
    (void) cfg;
    
    if (chip8->sp == 0) {
        fprintf(stderr, "%s: stack is empty\n", __func__);
        return -1;
    }

    *value = chip8->stack[--chip8->sp];
    return 0;
}

static int chip8_exec_unknown(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) chip8;
    (void) cfg;
    
    fprintf(stderr, "unknown instruction %04"PRIx16"\n", instruction);
    return -1;
}

// Clear the screen
static int chip8_exec_00E0(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    (void) instruction;
    
    memset(chip8->display, 0, sizeof(chip8->display));
    chip8->pc += 2;
    return 0;
}

// Return from subroutine
static int chip8_exec_00EE(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    (void) instruction;
    
    uint16_t return_address;
    if (chip8_stack_pop(chip8, cfg, &return_address) == -1) {
        return -1;
    }
    chip8->pc = return_address;
    return 0;
}

// Jump to address
static int chip8_exec_1NNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint16_t NNN = bits16(instruction, 12, 0);
    chip8->pc = NNN;
    return 0;
}

// Call subroutine at address
static int chip8_exec_2NNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    if (chip8_stack_push(chip8, cfg, chip8->pc + 2) == -1) {
        return -1;
    }

    uint16_t NNN = bits16(instruction, 12, 0);
    chip8->pc = NNN;
    return 0;
}

// Skip the next instruction if Vx == NN
static int chip8_exec_3XNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint16_t NN = bits16(instruction, 8, 0);

    if (chip8->registers[x] == NN) {
        chip8->pc += 2;
    }
    chip8->pc += 2;
    return 0;
}

// Skip the next instruction if Vx != NN
static int chip8_exec_4XNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint16_t NN = bits16(instruction, 8, 0);

    if (chip8->registers[x] != NN) {
        chip8->pc += 2;
    }
    chip8->pc += 2;
    return 0;
}

// Skip the next instruction if Vx == Vy
static int chip8_exec_5XY0(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);

    if (chip8->registers[x] == chip8->registers[y]) {
        chip8->pc += 2;
    }
    chip8->pc += 2;
    return 0;
}

// Set Vx = NN
static int chip8_exec_6XNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint16_t NN = bits16(instruction, 8, 0);

    chip8->registers[x] = NN;
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx + NN
static int chip8_exec_7XNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint16_t NN = bits16(instruction, 8, 0);

    chip8->registers[x] += NN;
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vy
static int chip8_exec_8XY0(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    
    chip8->registers[x] = chip8->registers[y];
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx | Vy
static int chip8_exec_8XY1(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    
    chip8->registers[x] |= chip8->registers[y];
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx & Vy
static int chip8_exec_8XY2(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    
    chip8->registers[x] &= chip8->registers[y];
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx ^ Vy
static int chip8_exec_8XY3(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    
    chip8->registers[x] ^= chip8->registers[y];
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx + Vy
// Set VF = 1 if the operation overflows and 0 otherwise
static int chip8_exec_8XY4(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    uint8_t VF = (chip8->registers[x] > UINT8_MAX - chip8->registers[y]);
    
    chip8->registers[x] += chip8->registers[y];
    chip8->registers[0xF] = VF;
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx - Vy
// Set VF = 0 if the operation underflows and 1 otherwise
static int chip8_exec_8XY5(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    uint8_t VF = (chip8->registers[x] >= chip8->registers[y]);
    
    chip8->registers[x] -= chip8->registers[y];
    chip8->registers[0xF] = VF;
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx >> 1 or Vy >> 1 depending on configuration
// Set VF to the lowest bit of Vx or Vy
static int chip8_exec_8XY6(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t VF = (chip8->registers[x] & 1);

    chip8->registers[x] >>= 1;
    chip8->registers[0xF] = VF;
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vy - Vx
// Set VF = 0 if the operation underflows and 1 otherwise
static int chip8_exec_8XY7(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);
    uint8_t VF = (chip8->registers[y] >= chip8->registers[x]);

    chip8->registers[x] = chip8->registers[y] - chip8->registers[x];
    chip8->registers[0xF] = VF;
    chip8->pc += 2;
    return 0;
}

// Set Vx = Vx << 1 or Vy << 1 depending on configuration
// Set VF to the highest bit of Vx or Vy
static int chip8_exec_8XYE(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t VF = (chip8->registers[x] >> 7);

    chip8->registers[x] <<= 1;
    chip8->registers[0xF] = VF;
    chip8->pc += 2;
    return 0;
}

// Skip the next instruction if Vx != Vy
static int chip8_exec_9XY0(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t y = bits16(instruction, 4, 1);

    if (chip8->registers[x] != chip8->registers[y]) {
        chip8->pc += 2;
    }
    chip8->pc += 2;
    return 0;
}

// Set I = NNN
static int chip8_exec_ANNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint16_t NNN = bits16(instruction, 12, 0);

    chip8->index_register = NNN;
    chip8->pc += 2;
    return 0;
}

// Jump to address V0 + XNN or Vx + XNN depending on configuration
static int chip8_exec_BXNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t V0 = chip8->registers[0];
    uint16_t NNN = bits16(instruction, 12, 0);

    chip8->pc = V0 + NNN;
    return 0;
}

// Set Vx = NN & random number
static int chip8_exec_CXNN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t NN = bits16(instruction, 8, 0);

    chip8->registers[x] = rand() & NN;
    chip8->pc += 2;
    return 0;
}

// Draw 8xN sprite located at I to display coordinates (Vx, Vy)
// Set VF = 1 if any pixels are flipped from set to unset and 0 otherwise
// The sprite is wrapped around if the coordinates are offscreen and clipped if they are near the edge
static int chip8_exec_DXYN(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;

    uint8_t Vx = chip8->registers[bits16(instruction, 4, 2)] % DISPLAY_WIDTH;
    uint8_t Vy = chip8->registers[bits16(instruction, 4, 1)] % DISPLAY_HEIGHT;

    uint8_t Vx_bytes = Vx / 8;
    uint8_t Vx_bits = Vx % 8;
    uint8_t n = bits16(instruction, 4, 0);
    uint8_t limit = (n > DISPLAY_HEIGHT - Vy) ? DISPLAY_HEIGHT - Vy : n;

    chip8->registers[0xF] = 0;

    for (uint8_t i = 0; i < limit; i++) {
        uint8_t prev;
        uint8_t next = chip8->memory[chip8->index_register + i];

        uint8_t index = (Vy + i) * DISPLAY_WIDTH / 8 + Vx_bytes;

        if (Vx_bits == 0) {
            prev = chip8->display[index];
            chip8->display[index] ^= next;
        } else {
            uint8_t prev1 = chip8->display[index];
            chip8->display[index] ^= (next >> Vx_bits);

            uint8_t prev2 = 0;
            if (Vx_bytes < DISPLAY_WIDTH / 8 - 1) {
                prev2 = chip8->display[index + 1];
                chip8->display[index + 1] ^= (next << (8 - Vx_bits));
            }

            prev = (prev1 << Vx_bits) | (prev2 >> (8 - Vx_bits));
        }

        if (~(prev ^ next) & prev) {
            chip8->registers[0xF] = 1;
        }
    }

    chip8->pc += 2;
    return 0;
}

// Skip the next instruction if the key stored in Vx is pressed
static int chip8_exec_EX9E(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t Vx = chip8->registers[x];
    uint8_t key = Vx & 0xF;

    if (chip8->keys[key]) {
        chip8->pc += 2;
    }
    chip8->pc += 2;
    return 0;
}

// Skip the next instruction if the key stored in Vx is not pressed
static int chip8_exec_EXA1(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t Vx = chip8->registers[x];
    uint8_t key = Vx & 0xF;

    if (!chip8->keys[key]) {
        chip8->pc += 2;
    }
    chip8->pc += 2;
    return 0;
}

// Set Vx to the value of the delay timer
static int chip8_exec_FX07(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);

    chip8->registers[x] = chip8->delay_timer;
    chip8->pc += 2;
    return 0;
}

// Wait for the next key press and store the pressed key in Vx
static int chip8_exec_FX0A(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;

    static bool keys[16] = {false};
    
    uint8_t x = bits16(instruction, 4, 2);

    for (int i = 0; i < 16; i++) {
        if (chip8->keys[i]) {
            keys[i] = true;
        } else if (keys[i]) {
            memset(keys, 0, sizeof(keys));
            chip8->registers[x] = i;
            chip8->pc += 2;
            break;
        }
    }

    return 0;
}

// Set the delay timer to Vx
static int chip8_exec_FX15(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);

    chip8->delay_timer = chip8->registers[x];
    chip8->pc += 2;
    return 0;
}

// Set the sound timer to Vx
static int chip8_exec_FX18(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);

    chip8->sound_timer = chip8->registers[x];
    chip8->pc += 2;
    return 0;
}

// Set I = I + Vx
static int chip8_exec_FX1E(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);

    chip8->index_register += chip8->registers[x];
    chip8->pc += 2;
    return 0;
}

// Set I to the location of the sprite for the character in Vx
static int chip8_exec_FX29(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t Vx = chip8->registers[x];
    uint8_t ch = Vx & 0xF;

    chip8->index_register = FONT_OFFSET + (FONT_LENGTH / 16) * ch;
    chip8->pc += 2;
    return 0;
}

// Store the binary-coded decimal representation of Vx at I
static int chip8_exec_FX33(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    uint8_t Vx = chip8->registers[x];

    for (int i = 2; i >= 0; i--) {
        chip8->memory[chip8->index_register + i] = Vx % 10;
        Vx /= 10;
    }

    chip8->pc += 2;
    return 0;
}

// Store the registers V0 to Vx in memory starting from I
// May increase I by x + 1 depending on configuration
static int chip8_exec_FX55(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);

    memcpy(&chip8->memory[chip8->index_register], chip8->registers, (x + 1) * sizeof(*chip8->registers));
    chip8->pc += 2;
    return 0;
}

// Read the registers V0 to Vx from memory starting at I
// May increase I by x + 1 depending on configuration
static int chip8_exec_FX65(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    (void) cfg;
    
    uint8_t x = bits16(instruction, 4, 2);
    
    memcpy(chip8->registers, &chip8->memory[chip8->index_register], (x + 1) * sizeof(*chip8->registers));
    chip8->pc += 2;
    return 0;
}

int chip8_exec(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction) {
    switch (bits16(instruction, 4, 3)) {
        case 0x0:
            switch (bits16(instruction, 12, 0)) {
                case 0x0E0: return chip8_exec_00E0(chip8, cfg, instruction);
                case 0x0EE: return chip8_exec_00EE(chip8, cfg, instruction);
                default: return chip8_exec_unknown(chip8, cfg, instruction);
            }
        case 0x1: return chip8_exec_1NNN(chip8, cfg, instruction);
        case 0x2: return chip8_exec_2NNN(chip8, cfg, instruction);
        case 0x3: return chip8_exec_3XNN(chip8, cfg, instruction);
        case 0x4: return chip8_exec_4XNN(chip8, cfg, instruction);
        case 0x5:
            if (bits16(instruction, 4, 0) == 0) {
                return chip8_exec_5XY0(chip8, cfg, instruction);
            } else {
                return chip8_exec_unknown(chip8, cfg, instruction);
            }
        case 0x6: return chip8_exec_6XNN(chip8, cfg, instruction);
        case 0x7: return chip8_exec_7XNN(chip8, cfg, instruction);
        case 0x8:
            switch (bits16(instruction, 4, 0)) {
                case 0x0: return chip8_exec_8XY0(chip8, cfg, instruction);
                case 0x1: return chip8_exec_8XY1(chip8, cfg, instruction);
                case 0x2: return chip8_exec_8XY2(chip8, cfg, instruction);
                case 0x3: return chip8_exec_8XY3(chip8, cfg, instruction);
                case 0x4: return chip8_exec_8XY4(chip8, cfg, instruction);
                case 0x5: return chip8_exec_8XY5(chip8, cfg, instruction);
                case 0x6: return chip8_exec_8XY6(chip8, cfg, instruction);
                case 0x7: return chip8_exec_8XY7(chip8, cfg, instruction);
                case 0xE: return chip8_exec_8XYE(chip8, cfg, instruction);
                default: return chip8_exec_unknown(chip8, cfg, instruction);
            }
        case 0x9:
            if (bits16(instruction, 4, 0) == 0) {
                return chip8_exec_9XY0(chip8, cfg, instruction);
            } else {
                return chip8_exec_unknown(chip8, cfg, instruction);
            }
        case 0xA: return chip8_exec_ANNN(chip8, cfg, instruction);
        case 0xB: return chip8_exec_BXNN(chip8, cfg, instruction);
        case 0xC: return chip8_exec_CXNN(chip8, cfg, instruction);
        case 0xD: return chip8_exec_DXYN(chip8, cfg, instruction);
        case 0xE:
            switch (bits16(instruction, 8, 0)) {
                case 0x9E: return chip8_exec_EX9E(chip8, cfg, instruction);
                case 0xA1: return chip8_exec_EXA1(chip8, cfg, instruction);
                default: return chip8_exec_unknown(chip8, cfg, instruction);
            }
        case 0xF:
            switch (bits16(instruction, 8, 0)) {
                case 0x07: return chip8_exec_FX07(chip8, cfg, instruction);
                case 0x0A: return chip8_exec_FX0A(chip8, cfg, instruction);
                case 0x15: return chip8_exec_FX15(chip8, cfg, instruction);
                case 0x18: return chip8_exec_FX18(chip8, cfg, instruction);
                case 0x1E: return chip8_exec_FX1E(chip8, cfg, instruction);
                case 0x29: return chip8_exec_FX29(chip8, cfg, instruction);
                case 0x33: return chip8_exec_FX33(chip8, cfg, instruction);
                case 0x55: return chip8_exec_FX55(chip8, cfg, instruction);
                case 0x65: return chip8_exec_FX65(chip8, cfg, instruction);
                default: return chip8_exec_unknown(chip8, cfg, instruction);
            }
    }

    return -1;
}

bool chip8_should_draw(uint16_t instruction) {
    return (instruction == 0x00E0) || (bits16(instruction, 4, 3) == 0xD);
}
