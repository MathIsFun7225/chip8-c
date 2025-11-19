#define _GNU_SOURCE

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "chip8.h"
#include "chip8_audio.h"
#include "chip8_config.h"
#include "chip8_display.h"
#include "chip8_exec.h"
#include "chip8_state.h"

#define MAX_PROGRAM_LENGTH (sizeof(((struct chip8_state *) NULL)->memory) - PROGRAM_MEMORY_OFFSET)
#define PROGRAM_MEMORY_OFFSET 0x200

static uint64_t current_time_ns(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return 1000000000ull * (uint64_t) ts.tv_sec + (uint64_t) ts.tv_nsec;
}

static void wait_until(uint64_t ns) {
    uint64_t current_time = current_time_ns();
    if (current_time >= ns) {
        return;
    } else if (ns <= current_time + 100000) {
        while (current_time_ns() <= ns);
    } else {
        struct timespec ts = { .tv_sec = (ns - current_time) / 1000000000ull, .tv_nsec = (ns - current_time) % 1000000000ull };
        nanosleep(&ts, NULL);
    }
}

static int fread_all(uint8_t *buf, size_t length, FILE *f) {
    size_t read = 0;

    while (read < length) {
        size_t r = fread(&buf[read], sizeof(*buf), length - read, f);
        if (r == 0) {
            fprintf(stderr, "%s: fread failed\n", __func__);
            return -1;
        }
        read += r;
    }

    return 0;
}

int chip8_load_program(struct chip8_state *state, const struct chip8_config *config, const char *file) {
    (void) config;

    struct stat file_info;
    if (stat(file, &file_info) == -1) {
        fprintf(stderr, "%s: stat: %s\n", __func__, strerror(errno));
        return -1;
    }
    if (file_info.st_size > MAX_PROGRAM_LENGTH) {
        fprintf(stderr, "%s: program must not exceed %zu bytes\n", __func__, (size_t) MAX_PROGRAM_LENGTH);
        return -1;
    }

    FILE *f = fopen(file, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: fopen: %s\n", __func__, strerror(errno));
        return -1;
    }

    if (fread_all(&state->memory[PROGRAM_MEMORY_OFFSET], file_info.st_size, f) == -1) {
        fclose(f);
        return -1;
    }
    memset(&state->memory[PROGRAM_MEMORY_OFFSET + file_info.st_size], 0, MAX_PROGRAM_LENGTH - file_info.st_size);

    fclose(f);
    return 0;
}

int chip8_run(struct chip8_state *state, struct chip8_display *display, struct chip8_audio *audio, const struct chip8_config *config) {
    (void) config;

    int return_value = 0;
    bool should_continue = true;
    bool pause = false;

    uint64_t frame_number = 0;
    uint64_t num_instructions = 0;

    while (should_continue) {
        uint64_t next_time = current_time_ns() + 1000000000ull / 60;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_1: state->keys[0x1] = true; break;
                    case SDL_SCANCODE_2: state->keys[0x2] = true; break;
                    case SDL_SCANCODE_3: state->keys[0x3] = true; break;
                    case SDL_SCANCODE_4: state->keys[0xC] = true; break;
                    case SDL_SCANCODE_Q: state->keys[0x4] = true; break;
                    case SDL_SCANCODE_W: state->keys[0x5] = true; break;
                    case SDL_SCANCODE_E: state->keys[0x6] = true; break;
                    case SDL_SCANCODE_R: state->keys[0xD] = true; break;
                    case SDL_SCANCODE_A: state->keys[0x7] = true; break;
                    case SDL_SCANCODE_S: state->keys[0x8] = true; break;
                    case SDL_SCANCODE_D: state->keys[0x9] = true; break;
                    case SDL_SCANCODE_F: state->keys[0xE] = true; break;
                    case SDL_SCANCODE_Z: state->keys[0xA] = true; break;
                    case SDL_SCANCODE_X: state->keys[0x0] = true; break;
                    case SDL_SCANCODE_C: state->keys[0xB] = true; break;
                    case SDL_SCANCODE_V: state->keys[0xF] = true; break;
                    case SDL_SCANCODE_P: pause = !pause; break;
                    case SDL_SCANCODE_ESCAPE: should_continue = false; break;
                    default: break;
                }
            }

            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_1: state->keys[0x1] = false; break;
                    case SDL_SCANCODE_2: state->keys[0x2] = false; break;
                    case SDL_SCANCODE_3: state->keys[0x3] = false; break;
                    case SDL_SCANCODE_4: state->keys[0xC] = false; break;
                    case SDL_SCANCODE_Q: state->keys[0x4] = false; break;
                    case SDL_SCANCODE_W: state->keys[0x5] = false; break;
                    case SDL_SCANCODE_E: state->keys[0x6] = false; break;
                    case SDL_SCANCODE_R: state->keys[0xD] = false; break;
                    case SDL_SCANCODE_A: state->keys[0x7] = false; break;
                    case SDL_SCANCODE_S: state->keys[0x8] = false; break;
                    case SDL_SCANCODE_D: state->keys[0x9] = false; break;
                    case SDL_SCANCODE_F: state->keys[0xE] = false; break;
                    case SDL_SCANCODE_Z: state->keys[0xA] = false; break;
                    case SDL_SCANCODE_X: state->keys[0x0] = false; break;
                    case SDL_SCANCODE_C: state->keys[0xB] = false; break;
                    case SDL_SCANCODE_V: state->keys[0xF] = false; break;
                    default: break;
                }
            }
            
            if (event.type == SDL_QUIT) {
                should_continue = false;
            }
        }

        if (!pause) {
            while (num_instructions < config->target_speed * ((frame_number % 60) + 1) / 60) {
                if (chip8_advance_state(state, config) == -1) {
                    should_continue = false;
                    return_value = -1;
                    break;
                }
                num_instructions++;
            }

            if (state->delay_timer > 0) {
                state->delay_timer--;
            }

            if (state->sound_timer > 0) {
                state->sound_timer--;
            }
        }

        if (chip8_update_display(display, state, config) == -1) {
            should_continue = false;
            return_value = -1;
        }

        if (chip8_update_audio(audio, state, config) == -1) {
            should_continue = false;
            return_value = -1;
        }

        frame_number++;
        num_instructions = 0;
        wait_until(next_time);
    }

    return return_value;
}
