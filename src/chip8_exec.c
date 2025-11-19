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
    
    fprintf(stderr, "unknown opcode 0x%04"PRIx16" at pc 0x%04"PRIx16"\n", opcode, state->pc);
    return -1;
}

// Scroll up by N pixels (N/2 in low-resolution mode)
static int chip8_exec_00BN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    uint8_t N = get_n(opcode);

    memmove(state->screen, &state->screen[N * DISPLAY_WIDTH / 8], (DISPLAY_HEIGHT - N) * DISPLAY_WIDTH / 8);
    memset(&state->screen[(DISPLAY_HEIGHT - N) * DISPLAY_WIDTH / 8], 0, N * DISPLAY_WIDTH / 8);

    state->pc += 2;
    return 0;
}

// Scroll down by N pixels (N/2 in low-resolution mode)
static int chip8_exec_00CN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    uint8_t N = get_n(opcode);

    memmove(&state->screen[N * DISPLAY_WIDTH / 8], state->screen, (DISPLAY_HEIGHT - N) * DISPLAY_WIDTH / 8);
    memset(state->screen, 0, N * DISPLAY_WIDTH / 8);

    state->pc += 2;
    return 0;
}

// Scroll up by N pixels (N/2 in low-resolution mode)
static int chip8_exec_00DN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    return chip8_exec_00BN(state, config, opcode);
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
    (void) opcode;

    return chip8_stack_pop(state, config, &state->pc);
}

// Scroll right by 4 pixels (2 in low-resolution mode)
static int chip8_exec_00FB(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;

    for (uint8_t i = 0; i < DISPLAY_HEIGHT; i++) {
        for (uint8_t j = DISPLAY_WIDTH / 8 - 1; j >= 1; j--) {
            uint16_t index = i * DISPLAY_WIDTH / 8 + j;
            state->screen[index] = (state->screen[index] >> 4) | (state->screen[index - 1] << 4);
        }
        state->screen[i * DISPLAY_WIDTH / 8] >>= 4;
    }
    state->pc += 2;
    return 0;
}

// Scroll left by 4 pixels (2 in low-resolution mode)
static int chip8_exec_00FC(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;
    
    for (uint8_t i = 0; i < DISPLAY_HEIGHT; i++) {
        for (uint8_t j = 0; j < DISPLAY_WIDTH / 8 - 1; j++) {
            uint16_t index = i * DISPLAY_WIDTH / 8 + j;
            state->screen[index] = (state->screen[index] << 4) | (state->screen[index + 1] >> 4);
        }
        state->screen[(i + 1) * DISPLAY_WIDTH / 8 - 1] <<= 4;
    }
    state->pc += 2;
    return 0;
}

// Exit interpreter
static int chip8_exec_00FD(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;

    state->stopped = true;
    state->pc += 2;
    return 0;
}

// Disable high-resolution
static int chip8_exec_00FE(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;

    state->hires = false;
    state->pc += 2;
    return 0;
}

// Enable high-resolution
static int chip8_exec_00FF(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    (void) opcode;

    state->hires = true;
    state->pc += 2;
    return 0;
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

static const uint8_t lores_lookup[] = {
    0x00, 0x03, 0x0C, 0x0F,
    0x30, 0x33, 0x3C, 0x3F,
    0xC0, 0xC3, 0xCC, 0xCF,
    0xF0, 0xF3, 0xFC, 0xFF
};

// Helper function to draw a single byte (8x1 block)
static void chip8_apply_mask(struct chip8_state *state, uint8_t x_bytes, uint8_t x_bits, uint8_t y, uint8_t mask) {
    uint16_t index = (DISPLAY_WIDTH / 8) * y + x_bytes;

    uint8_t prev;
    if (x_bits == 0) {
        prev = state->screen[index];
        state->screen[index] ^= mask;
    } else {
        uint8_t prev1 = state->screen[index] << x_bits;
        state->screen[index] ^= (mask >> x_bits);

        uint8_t prev2 = 0;
        if (x_bytes < DISPLAY_WIDTH / 8 - 1) {
            prev2 = state->screen[index + 1] >> (8 - x_bits);
            state->screen[index + 1] ^= (mask << (8 - x_bits));
        }

        prev = prev1 | prev2;
    }

    if (~(prev ^ mask) & prev) {
        state->registers[0xF] = 1;
    }
}

// Draw 8xN sprite (16x16 in hires if N = 0) located at I to display coordinates (Vx, Vy)
// Set VF = 1 if any pixels are flipped from set to unset and 0 otherwise
// The sprite is wrapped around if the coordinates are offscreen and clipped if they are near the edge
static int chip8_exec_DXYN(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    // const uint8_t w = DISPLAY_WIDTH / 2;
    // const uint8_t h = DISPLAY_HEIGHT / 2;

    // uint8_t Vx = state->registers[get_x(opcode)] % w;
    // uint8_t Vy = state->registers[get_y(opcode)] % h;

    // uint8_t Vx_bytes = Vx / 8;
    // uint8_t Vx_bits = Vx % 8;
    // uint8_t n = get_n(opcode);
    // uint8_t limit = (n > h - Vy) ? h - Vy : n;

    // state->registers[0xF] = 0;

    // for (uint8_t i = 0; i < limit; i++) {
    //     uint8_t prev;
    //     uint8_t mask = state->memory[state->index_register + i];

    //     uint16_t index = (Vy + i) * DISPLAY_WIDTH / 8 + Vx_bytes;

    //     if (Vx_bits == 0) {
    //         prev = state->screen[index];
    //         state->screen[index] ^= mask;
    //     } else {
    //         uint8_t prev1 = state->screen[index];
    //         state->screen[index] ^= (mask >> Vx_bits);

    //         uint8_t prev2 = 0;
    //         if (Vx_bytes < w / 8 - 1) {
    //             prev2 = state->screen[index + 1];
    //             state->screen[index + 1] ^= (mask << (8 - Vx_bits));
    //         }

    //         prev = (prev1 << Vx_bits) | (prev2 >> (8 - Vx_bits));
    //     }

    //     // A pixel was flipped from set to unset
    //     if (~(prev ^ mask) & prev) {
    //         state->registers[0xF] = 1;
    //     }
    // }

    uint8_t scale = state->hires ? 1 : 2;

    uint8_t Vx = (scale * state->registers[get_x(opcode)]) % DISPLAY_WIDTH;
    uint8_t Vy = (scale * state->registers[get_y(opcode)]) % DISPLAY_HEIGHT;

    uint8_t Vx_bytes = Vx / 8;
    uint8_t Vx_bits = Vx % 8;

    uint8_t N = get_n(opcode);

    uint8_t cols = (N == 0 && state->hires) ? 16 : 8;
    uint8_t rows = (N == 0 && state->hires) ? 16 : N;

    uint8_t x_limit = (cols * scale > DISPLAY_WIDTH - Vx) ? (DISPLAY_WIDTH - Vx) / scale : cols;
    cols /= 8;
    x_limit = (x_limit + 7) / 8;
    uint8_t y_limit = (rows * scale > DISPLAY_HEIGHT - Vy) ? (DISPLAY_HEIGHT - Vy) / scale : rows;

    state->registers[0xF] = 0;

    for (uint8_t j = 0; j < y_limit; j++) {
        for (uint8_t i = 0; i < x_limit; i++) {
            uint8_t mask = state->memory[state->index_register + j * cols + i];

            if (state->hires) {
                chip8_apply_mask(state, Vx_bytes + i, Vx_bits, Vy + j, mask);
            } else {
                chip8_apply_mask(state, Vx_bytes + 2 * i,     Vx_bits, Vy + 2 * j,     lores_lookup[mask >> 4]);
                chip8_apply_mask(state, Vx_bytes + 2 * i + 1, Vx_bits, Vy + 2 * j,     lores_lookup[mask & 0xF]);
                chip8_apply_mask(state, Vx_bytes + 2 * i,     Vx_bits, Vy + 2 * j + 1, lores_lookup[mask >> 4]);
                chip8_apply_mask(state, Vx_bytes + 2 * i + 1, Vx_bits, Vy + 2 * j + 1, lores_lookup[mask & 0xF]);
            }
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
    
    uint8_t x = get_x(opcode);
    for (int i = 0; i < 16; i++) {
        if (state->keys[i]) {
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
    state->index_register &= 0xFFF;
    state->pc += 2;
    return 0;
}

// Set I to the location of the 5-byte sprite for the character in Vx
static int chip8_exec_FX29(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t Vx = state->registers[x];
    uint8_t ch = Vx & 0xF;

    state->index_register = FONT_MEMORY_OFFSET + (LORES_FONT_LENGTH / 16) * ch;
    state->pc += 2;
    return 0;
}

// Set I to the location of the 10-byte sprite for the character in Vx
static int chip8_exec_FX30(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;
    
    uint8_t x = get_x(opcode);
    uint8_t Vx = state->registers[x];
    uint8_t ch = Vx & 0xF;

    state->index_register = FONT_MEMORY_OFFSET + LORES_FONT_LENGTH + (HIRES_FONT_LENGTH / 16) * ch;
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

// Store registers V0 to Vx in RPL user flags
static int chip8_exec_FX75(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    uint8_t x = get_x(opcode);
    memcpy(state->rpl_flags, state->registers, x + 1);
    state->pc += 2;
    return 0;
}

// Read registers V0 to Vx from RPL user flags
static int chip8_exec_FX85(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    (void) config;

    uint8_t x = get_x(opcode);
    memcpy(state->registers, state->rpl_flags, x + 1);
    state->pc += 2;
    return 0;
}

int chip8_exec(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode) {
    switch (get_h(opcode)) {
        case 0x0:
            switch (get_nnn(opcode) >> 4) {
                case 0x0B: return chip8_exec_00BN(state, config, opcode);
                case 0x0C: return chip8_exec_00CN(state, config, opcode);
                case 0x0D: return chip8_exec_00DN(state, config, opcode);
                default:
                    switch (get_nn(opcode)) {
                        case 0xE0: return chip8_exec_00E0(state, config, opcode);
                        case 0xEE: return chip8_exec_00EE(state, config, opcode);
                        case 0xFB: return chip8_exec_00FB(state, config, opcode);
                        case 0xFC: return chip8_exec_00FC(state, config, opcode);
                        case 0xFD: return chip8_exec_00FD(state, config, opcode);
                        case 0xFE: return chip8_exec_00FE(state, config, opcode);
                        case 0xFF: return chip8_exec_00FF(state, config, opcode);
                        default: return chip8_exec_unknown(state, config, opcode);
                    }
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
                case 0x30: return chip8_exec_FX30(state, config, opcode);
                case 0x33: return chip8_exec_FX33(state, config, opcode);
                case 0x55: return chip8_exec_FX55(state, config, opcode);
                case 0x65: return chip8_exec_FX65(state, config, opcode);
                case 0x75: return chip8_exec_FX75(state, config, opcode);
                case 0x85: return chip8_exec_FX85(state, config, opcode);
                default: return chip8_exec_unknown(state, config, opcode);
            }
    }

    return -1;
}
