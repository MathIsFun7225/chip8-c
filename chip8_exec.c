#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8_config.h"
#include "chip8_exec.h"
#include "chip8_state.h"

static int chip8_exec_unknown(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) state;
    (void) config;
    
    fprintf(stderr, "unknown opcode 0x%04"PRIx16"\n", opcode);
    return -1;
}

// Clear the screen
static int chip8_exec_00E0(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;
    
    memset(state->screen, 0, sizeof(state->screen));
    state->pc += 2;
    return 0;
}

// Return from subroutine
static int chip8_exec_00EE(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;

    return chip8_stack_pop(state, config, &state->pc);
}

// Jump to address NNN
static int chip8_exec_1NNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint16_t NNN = get_nnn(opcode);
    state->pc = NNN;
    return 0;
}

// Call subroutine at address NNN
static int chip8_exec_2NNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    if (chip8_stack_push(state, config, state->pc + 2) == -1) {
        return -1;
    }

    uint16_t NNN = get_nnn(opcode);
    state->pc = NNN;
    return 0;
}

// Skip the next instruction if Vx == NN
static int chip8_exec_3XNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint16_t NN = get_nn(opcode);

    if (state->registers[x] == NN) {
        state->pc += 2;
    }
    state->pc += 2;
    return 0;
}

// Skip the next instruction if Vx != NN
static int chip8_exec_4XNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint16_t NN = get_nn(opcode);

    if (state->registers[x] != NN) {
        state->pc += 2;
    }
    state->pc += 2;
    return 0;
}

// Skip the next instruction if Vx == Vy
static int chip8_exec_5XY0(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);

    if (state->registers[x] == state->registers[y]) {
        state->pc += 2;
    }
    state->pc += 2;
    return 0;
}

// Set Vx = NN
static int chip8_exec_6XNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint16_t NN = get_nn(opcode);

    state->registers[x] = NN;
    state->pc += 2;
    return 0;
}

// Set Vx = Vx + NN
static int chip8_exec_7XNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint16_t NN = get_nn(opcode);

    state->registers[x] += NN;
    state->pc += 2;
    return 0;
}

// Set Vx = Vy
static int chip8_exec_8XY0(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    
    state->registers[x] = state->registers[y];
    state->pc += 2;
    return 0;
}

// Set Vx = Vx | Vy
static int chip8_exec_8XY1(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    
    state->registers[x] |= state->registers[y];
    state->pc += 2;
    return 0;
}

// Set Vx = Vx & Vy
static int chip8_exec_8XY2(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    
    state->registers[x] &= state->registers[y];
    state->pc += 2;
    return 0;
}

// Set Vx = Vx ^ Vy
static int chip8_exec_8XY3(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    
    state->registers[x] ^= state->registers[y];
    state->pc += 2;
    return 0;
}

// Set Vx = Vx + Vy
// Set VF = 1 if the operation overflows and 0 otherwise
static int chip8_exec_8XY4(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    uint8_t VF = (state->registers[x] > UINT8_MAX - state->registers[y]);
    
    state->registers[x] += state->registers[y];
    state->registers[0xF] = VF;
    state->pc += 2;
    return 0;
}

// Set Vx = Vx - Vy
// Set VF = 0 if the operation underflows and 1 otherwise
static int chip8_exec_8XY5(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    uint8_t VF = (state->registers[x] >= state->registers[y]);
    
    state->registers[x] -= state->registers[y];
    state->registers[0xF] = VF;
    state->pc += 2;
    return 0;
}

// Set Vx = Vx >> 1 or Vy >> 1 depending on configuration
// Set VF to the lowest bit of Vx or Vy
static int chip8_exec_8XY6(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t VF = (state->registers[x] & 1);

    state->registers[x] >>= 1;
    state->registers[0xF] = VF;
    state->pc += 2;
    return 0;
}

// Set Vx = Vy - Vx
// Set VF = 0 if the operation underflows and 1 otherwise
static int chip8_exec_8XY7(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    uint8_t VF = (state->registers[y] >= state->registers[x]);

    state->registers[x] = state->registers[y] - state->registers[x];
    state->registers[0xF] = VF;
    state->pc += 2;
    return 0;
}

// Set Vx = Vx << 1 or Vy << 1 depending on configuration
// Set VF to the highest bit of Vx or Vy
static int chip8_exec_8XYE(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t VF = (state->registers[x] >> 7);

    state->registers[x] <<= 1;
    state->registers[0xF] = VF;
    state->pc += 2;
    return 0;
}

// Skip the next instruction if Vx != Vy
static int chip8_exec_9XY0(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);

    if (state->registers[x] != state->registers[y]) {
        state->pc += 2;
    }
    state->pc += 2;
    return 0;
}

// Set I = NNN
static int chip8_exec_ANNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint16_t NNN = get_nnn(opcode);

    state->index_register = NNN;
    state->pc += 2;
    return 0;
}

// Jump to address V0 + XNN or Vx + XNN depending on configuration
static int chip8_exec_BXNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t V0 = state->registers[0];
    uint16_t NNN = get_nnn(opcode);

    state->pc = V0 + NNN;
    return 0;
}

// Set Vx = NN & random number
static int chip8_exec_CXNN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t NN = get_nn(opcode);

    state->registers[x] = rand() & NN;
    state->pc += 2;
    return 0;
}

// Draw 8xN sprite located at I to display coordinates (Vx, Vy)
// Set VF = 1 if any pixels are flipped from set to unset and 0 otherwise
// The sprite is wrapped around if the coordinates are offscreen and clipped if they are near the edge
static int chip8_exec_DXYN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    uint8_t Vx = state->registers[get_x(opcode)] % DISPLAY_WIDTH;
    uint8_t Vy = state->registers[get_y(opcode)] % DISPLAY_HEIGHT;

    uint8_t Vx_bytes = Vx / 8;
    uint8_t Vx_bits = Vx % 8;
    uint8_t n = get_n(opcode);
    uint8_t limit = (n > DISPLAY_HEIGHT - Vy) ? DISPLAY_HEIGHT - Vy : n;

    state->registers[0xF] = 0;

    for (uint8_t i = 0; i < limit; i++) {
        uint8_t prev;
        uint8_t mask = state->memory[state->index_register + i];

        uint8_t index = (Vy + i) * DISPLAY_WIDTH / 8 + Vx_bytes;

        if (Vx_bits == 0) {
            prev = state->screen[index];
            state->screen[index] ^= mask;
        } else {
            uint8_t prev1 = state->screen[index];
            state->screen[index] ^= (mask >> Vx_bits);

            uint8_t prev2 = 0;
            if (Vx_bytes < DISPLAY_WIDTH / 8 - 1) {
                prev2 = state->screen[index + 1];
                state->screen[index + 1] ^= (mask << (8 - Vx_bits));
            }

            prev = (prev1 << Vx_bits) | (prev2 >> (8 - Vx_bits));
        }

        // A pixel was flipped from set to unset
        if (~(prev ^ mask) & prev) {
            state->registers[0xF] = 1;
        }
    }

    state->pc += 2;
    return 0;
}

// Skip the next instruction if the key stored in Vx is pressed
static int chip8_exec_EX9E(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t Vx = state->registers[x];
    uint8_t key = Vx & 0xF;

    if (state->keys[key]) {
        state->pc += 2;
    }
    state->pc += 2;
    return 0;
}

// Skip the next instruction if the key stored in Vx is not pressed
static int chip8_exec_EXA1(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t Vx = state->registers[x];
    uint8_t key = Vx & 0xF;

    if (!state->keys[key]) {
        state->pc += 2;
    }
    state->pc += 2;
    return 0;
}

// Set Vx to the value of the delay timer
static int chip8_exec_FX07(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);

    state->registers[x] = state->delay_timer;
    state->pc += 2;
    return 0;
}

// Wait for the next key press and store the pressed key in Vx
static int chip8_exec_FX0A(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    static bool keys[16] = {false};
    
    uint8_t x = get_x(opcode);

    for (int i = 0; i < 16; i++) {
        if (state->keys[i]) {
            keys[i] = true;
        } else if (keys[i]) {
            memset(keys, 0, sizeof(keys));
            state->registers[x] = i;
            state->pc += 2;
            break;
        }
    }

    return 0;
}

// Set the delay timer to Vx
static int chip8_exec_FX15(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);

    state->delay_timer = state->registers[x];
    state->pc += 2;
    return 0;
}

// Set the sound timer to Vx
static int chip8_exec_FX18(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);

    state->sound_timer = state->registers[x];
    state->pc += 2;
    return 0;
}

// Set I = I + Vx
static int chip8_exec_FX1E(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);

    state->index_register += state->registers[x];
    state->pc += 2;
    return 0;
}

// Set I to the location of the sprite for the character in Vx
static int chip8_exec_FX29(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t Vx = state->registers[x];
    uint8_t ch = Vx & 0xF;

    state->index_register = FONT_MEMORY_OFFSET + (FONT_LENGTH / 16) * ch;
    state->pc += 2;
    return 0;
}

// Store the binary-coded decimal representation of Vx at I
static int chip8_exec_FX33(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t Vx = state->registers[x];

    for (int i = 2; i >= 0; i--) {
        state->memory[state->index_register + i] = Vx % 10;
        Vx /= 10;
    }

    state->pc += 2;
    return 0;
}

// Store the registers V0 to Vx in memory starting from I
// May increase I by x + 1 depending on configuration
static int chip8_exec_FX55(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);

    memcpy(&state->memory[state->index_register], state->registers, (x + 1) * sizeof(*state->registers));
    state->pc += 2;
    return 0;
}

// Read the registers V0 to Vx from memory starting at I
// May increase I by x + 1 depending on configuration
static int chip8_exec_FX65(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    
    memcpy(state->registers, &state->memory[state->index_register], (x + 1) * sizeof(*state->registers));
    state->pc += 2;
    return 0;
}

int chip8_exec(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    switch (get_h(opcode)) {
        case 0x0:
            switch (get_nnn(opcode)) {
                case 0x0E0: return chip8_exec_00E0(state, config, opcode);
                case 0x0EE: return chip8_exec_00EE(state, config, opcode);
                default: return chip8_exec_unknown(state, config, opcode);
            }
        case 0x1: return chip8_exec_1NNN(state, config, opcode);
        case 0x2: return chip8_exec_2NNN(state, config, opcode);
        case 0x3: return chip8_exec_3XNN(state, config, opcode);
        case 0x4: return chip8_exec_4XNN(state, config, opcode);
        case 0x5:
            if (get_n(opcode) == 0) {
                return chip8_exec_5XY0(state, config, opcode);
            } else {
                return chip8_exec_unknown(state, config, opcode);
            }
        case 0x6: return chip8_exec_6XNN(state, config, opcode);
        case 0x7: return chip8_exec_7XNN(state, config, opcode);
        case 0x8:
            switch (get_n(opcode)) {
                case 0x0: return chip8_exec_8XY0(state, config, opcode);
                case 0x1: return chip8_exec_8XY1(state, config, opcode);
                case 0x2: return chip8_exec_8XY2(state, config, opcode);
                case 0x3: return chip8_exec_8XY3(state, config, opcode);
                case 0x4: return chip8_exec_8XY4(state, config, opcode);
                case 0x5: return chip8_exec_8XY5(state, config, opcode);
                case 0x6: return chip8_exec_8XY6(state, config, opcode);
                case 0x7: return chip8_exec_8XY7(state, config, opcode);
                case 0xE: return chip8_exec_8XYE(state, config, opcode);
                default: return chip8_exec_unknown(state, config, opcode);
            }
        case 0x9:
            if (get_n(opcode) == 0) {
                return chip8_exec_9XY0(state, config, opcode);
            } else {
                return chip8_exec_unknown(state, config, opcode);
            }
        case 0xA: return chip8_exec_ANNN(state, config, opcode);
        case 0xB: return chip8_exec_BXNN(state, config, opcode);
        case 0xC: return chip8_exec_CXNN(state, config, opcode);
        case 0xD: return chip8_exec_DXYN(state, config, opcode);
        case 0xE:
            switch (get_nn(opcode)) {
                case 0x9E: return chip8_exec_EX9E(state, config, opcode);
                case 0xA1: return chip8_exec_EXA1(state, config, opcode);
                default: return chip8_exec_unknown(state, config, opcode);
            }
        case 0xF:
            switch (get_nn(opcode)) {
                case 0x07: return chip8_exec_FX07(state, config, opcode);
                case 0x0A: return chip8_exec_FX0A(state, config, opcode);
                case 0x15: return chip8_exec_FX15(state, config, opcode);
                case 0x18: return chip8_exec_FX18(state, config, opcode);
                case 0x1E: return chip8_exec_FX1E(state, config, opcode);
                case 0x29: return chip8_exec_FX29(state, config, opcode);
                case 0x33: return chip8_exec_FX33(state, config, opcode);
                case 0x55: return chip8_exec_FX55(state, config, opcode);
                case 0x65: return chip8_exec_FX65(state, config, opcode);
                default: return chip8_exec_unknown(state, config, opcode);
            }
    }

    return -1;
}
