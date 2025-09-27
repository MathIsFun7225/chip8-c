#ifndef CHIP8_STATE_H
#define CHIP8_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "chip8_config.h"

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define FONT_MEMORY_OFFSET 0
#define FONT_LENGTH 0x50
#define PROGRAM_MEMORY_OFFSET 0x200

struct chip8_state {
    uint8_t memory[4096];

    uint8_t screen[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

    uint8_t registers[16];
    uint16_t index_register;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t pc;

    bool keys[16];

    uint16_t stack_size;
    uint16_t sp;
    uint16_t *stack;
};

struct chip8_state_compressed {
    uint8_t *compressed_memory;
    ssize_t compressed_memory_size;

    uint8_t *compressed_screen;
    ssize_t compressed_screen_size;

    uint8_t registers[16];
    uint16_t index_register;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t pc;

    bool keys[16];

    uint16_t stack_size;
    uint16_t sp;
    uint16_t *stack;
};

int chip8_stack_resize(struct chip8_state *state, const struct chip8_config *config, uint16_t new_size);
int chip8_stack_push(struct chip8_state *state, const struct chip8_config *config, uint16_t value);
int chip8_stack_pop(struct chip8_state *state, const struct chip8_config *config, uint16_t *value);

int chip8_init_state(struct chip8_state *state, const struct chip8_config *config);
int chip8_close_state(struct chip8_state *state, const struct chip8_config *config);

int chip8_dump_state(FILE *f, const struct chip8_state *state, const struct chip8_config *config);
int chip8_load_state(FILE *f, struct chip8_state *state, const struct chip8_config *config);

int chip8_advance_state(struct chip8_state *state, const struct chip8_config *config);
int chip8_rewind_state(struct chip8_state *state, const struct chip8_config *config);

#endif // CHIP8_STATE_H
