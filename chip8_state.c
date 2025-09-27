#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "chip8_config.h"
#include "chip8_exec.h"
#include "chip8_state.h"
#include "helper.h"

#define bits16(x, size, num) ((uint16_t) (((x) >> (num)) & ((1 << (size)) - 1)))
#define HISTORY_MAX (60 * 60)

//static struct chip8_state base_state;
//static struct chip8_state_compressed g_history[HISTORY_MAX];
//static ssize_t g_num_stored = 0;
//static ssize_t g_prev = -1;

int chip8_stack_resize(struct chip8_state *state, const struct chip8_config *config, uint16_t new_size) {
    (void) config;

    if (state->stack_size < new_size) {
        uint16_t *p = realloc(state->stack, new_size * sizeof(*state->stack));
        if (p == NULL) {
            return -1;
        }
        state->stack = p;
        state->stack_size = new_size;
    }
    return 0;
}

int chip8_stack_push(struct chip8_state *state, const struct chip8_config *config, uint16_t value) {
    if (state->stack_size == state->sp) {
        if (chip8_stack_resize(state, config, state->stack_size * 2) == -1) {
            return -1;
        }
    }

    state->stack[state->sp++] = value;
    return 0;
}

int chip8_stack_pop(struct chip8_state *state, const struct chip8_config *config, uint16_t *value) {
    (void) config;
    
    if (state->sp == 0) {
        fprintf(stderr, "%s: stack is empty\n", __func__);
        return -1;
    }

    *value = state->stack[--state->sp];
    return 0;
}

static void chip8_compress_state(struct chip8_state_compressed *compressed, const struct chip8_state *state, const struct chip8_config *config) {
    ;
}

static void chip8_decompress_state(struct chip8_state *state, const struct chip8_state_compressed *compressed, const struct chip8_config *config) {
    ;
}

static void chip8_history_add(struct chip8_state *state, const struct chip8_config *config) {
    ;
}

int chip8_init_state(struct chip8_state *state, const struct chip8_config *config) {
    (void) config;

    memset(state, 0, sizeof(*state));

    state->pc = PROGRAM_MEMORY_OFFSET;

    state->stack_size = 16;
    state->stack = malloc(state->stack_size * sizeof(*state->stack));
    if (state->stack == NULL) {
        return -1;
    }

    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80
    };
    memcpy(&state->memory[FONT_MEMORY_OFFSET], font, sizeof(font));

    return 0;
}

int chip8_close_state(struct chip8_state *state, const struct chip8_config *config) {
    free(state->stack);
    return 0;
}

int chip8_dump_state(FILE *f, const struct chip8_state *state, const struct chip8_config *config) {
    if (fwrite(&state->memory,    sizeof(state->memory),    1, f) == 0) return -1;
    if (fwrite(&state->screen,    sizeof(state->screen),    1, f) == 0) return -1;
    if (fwrite(&state->registers, sizeof(state->registers), 1, f) == 0) return -1;

    if (serialize_16(f, state->index_register) == -1) return -1;

    if (fwrite(&state->delay_timer, sizeof(state->delay_timer), 1, f) == 0) return -1;
    if (fwrite(&state->sound_timer, sizeof(state->sound_timer), 1, f) == 0) return -1;

    if (serialize_16(f, state->pc) == -1) return -1;
    
    if (fwrite(&state->keys, sizeof(state->keys), 1, f) == 0) return -1;

    if (serialize_16(f, state->stack_size) == -1) return -1;
    if (serialize_16(f, state->sp)         == -1) return -1;

    for (uint16_t i = 0; i < state->sp; i++) {
        if (serialize_16(f, state->stack[i]) == -1) {
            return -1;
        }
    }

    return 0;
}

int chip8_load_state(FILE *f, struct chip8_state *state, const struct chip8_config *config) {
    if (fread(&state->memory,    sizeof(state->memory),    1, f) == 0) return -1;
    if (fread(&state->screen,    sizeof(state->screen),    1, f) == 0) return -1;
    if (fread(&state->registers, sizeof(state->registers), 1, f) == 0) return -1;

    if (deserialize_16(f, &state->index_register) == -1) return -1;
    
    if (fread(&state->delay_timer, sizeof(state->delay_timer), 1, f) == 0) return -1;
    if (fread(&state->sound_timer, sizeof(state->sound_timer), 1, f) == 0) return -1;
    
    if (deserialize_16(f, &state->pc) == -1) return -1;
    
    if (fread(&state->keys, sizeof(state->keys), 1, f) == 0) return -1;

    uint16_t new_stack_size;
    if (deserialize_16(f, &new_stack_size) == -1) return -1;
    if (deserialize_16(f, &state->sp)      == -1) return -1;
    if (chip8_stack_resize(state, config, new_stack_size) == -1) return -1;

    for (uint16_t i = 0; i < state->sp; i++) {
        if (deserialize_16(f, &state->stack[i]) == -1) {
            return -1;
        }
    }

    return 0;
}

int chip8_advance_state(struct chip8_state *state, const struct chip8_config *config) {
    uint16_t instruction = ((uint16_t) state->memory[state->pc]) << 8 | state->memory[state->pc + 1];

    return chip8_exec(state, config, instruction);
}

int chip8_rewind_state(struct chip8_state *state, const struct chip8_config *config);
