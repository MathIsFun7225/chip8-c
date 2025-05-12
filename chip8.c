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

int chip8_init(struct chip8 *chip8, const struct chip8_config *cfg) {
    (void) cfg;

    memset(chip8, 0, sizeof(*chip8));
    chip8->pc = PROGRAM_OFFSET;
    chip8->stack = malloc(16 * sizeof(*chip8->stack));
    if (chip8->stack == NULL) {
        fprintf(stderr, "%s: malloc: %s\n", __func__, strerror(errno));
        return -1;
    }

    uint8_t font[] = {
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
    chip8_load_font_arr(chip8, cfg, font);

    return 0;
}

int chip8_init_sdl(struct chip8_display *disp, const struct chip8_config *cfg) {
    (void) cfg;

    SDL_Init(SDL_INIT_VIDEO);

    disp->window = SDL_CreateWindow(
        "chip8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        64 * cfg->default_scale, 32 * cfg->default_scale,
        // 64, 32,
        SDL_WINDOW_RESIZABLE
    );
    if (disp->window == NULL) {
        fprintf(stderr, "%s: SDL_CreateWindow: %s\n", __func__, SDL_GetError());
        return -1;
    }

    disp->renderer = SDL_CreateRenderer(disp->window, -1, 0);
    if (disp->renderer == NULL) {
        fprintf(stderr, "%s: SDL_CreateRenderer: %s\n", __func__, SDL_GetError());
        SDL_DestroyWindow(disp->window);
        return -1;
    }

    /*
    if (SDL_RenderSetScale(disp->renderer, cfg->default_scale, cfg->default_scale) < 0) {
        fprintf(stderr, "SDL_RenderSetScale(): %s\n", SDL_GetError());
        SDL_DestroyWindow(disp->window);
        SDL_DestroyRenderer(disp->renderer);
    }
    */

    if (SDL_RenderSetLogicalSize(disp->renderer, 64, 32) < 0) {
        fprintf(stderr, "SDL_RenderSetLogicalSize(): %s\n", SDL_GetError());
        SDL_DestroyWindow(disp->window);
        SDL_DestroyRenderer(disp->renderer);
        return -1;
    }

    if (SDL_RenderSetIntegerScale(disp->renderer, SDL_TRUE) < 0) {
        fprintf(stderr, "SDL_RenderSetIntegerScale(): %s\n", SDL_GetError());
        SDL_DestroyWindow(disp->window);
        SDL_DestroyRenderer(disp->renderer);
        return -1;
    }

    return 0;
}

int chip8_close(struct chip8 *chip8, const struct chip8_config *cfg) {
    (void) cfg;

    free(chip8->stack);
    memset(chip8, 0, sizeof(*chip8));

    return 0;
}

int chip8_close_sdl(struct chip8_display *disp, const struct chip8_config *cfg) {
    (void) cfg;

    SDL_DestroyRenderer(disp->renderer);
    SDL_DestroyWindow(disp->window);
    memset(disp, 0, sizeof(*disp));
    SDL_Quit();

    return 0;
}

int chip8_load_font_arr(struct chip8 *chip8, const struct chip8_config *cfg, const uint8_t *font) {
    (void) cfg;

    memcpy(chip8->memory, font, FONT_LENGTH);
    return 0;
}

int chip8_load_font_file(struct chip8 *chip8, const struct chip8_config *cfg, const char *file) {
    (void) cfg;

    struct stat file_info;
    if (stat(file, &file_info) == -1) {
        fprintf(stderr, "%s: stat: %s\n", __func__, strerror(errno));
        return -1;
    }
    if (file_info.st_size != FONT_LENGTH) {
        fprintf(stderr, "%s: font file length must be 200 bytes\n", __func__);
        return -1;
    }

    FILE *f = fopen(file, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: fopen: %s\n", __func__, strerror(errno));
        return -1;
    }

    int result = fread_all(&chip8->memory[0], FONT_LENGTH, f);
    fclose(f);
    return result;
}

int chip8_load_program_arr(struct chip8 *chip8, const struct chip8_config *cfg, const uint8_t *buf, uint16_t length) {
    (void) cfg;

    if (length > MAX_PROGRAM_LENGTH) {
        length = MAX_PROGRAM_LENGTH;
    }

    memcpy(&chip8->memory[PROGRAM_OFFSET], buf, length);
    memset(&chip8->memory[PROGRAM_OFFSET + length], 0, MAX_PROGRAM_LENGTH - length);

    return 0;
}


int chip8_load_program_file(struct chip8 *chip8, const struct chip8_config *cfg, const char *file) {
    (void) cfg;

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

    if (fread_all(&chip8->memory[PROGRAM_OFFSET], file_info.st_size, f) == -1) {
        fclose(f);
        return -1;
    }
    memset(&chip8->memory[PROGRAM_OFFSET + file_info.st_size], 0, MAX_PROGRAM_LENGTH - file_info.st_size);

    fclose(f);
    return 0;
}

uint16_t chip8_get_instruction(const struct chip8 *chip8, const struct chip8_config *cfg) {
    (void) cfg;

    return (((uint16_t) chip8->memory[chip8->pc]) << 8) + chip8->memory[chip8->pc + 1];
}

int chip8_draw(struct chip8_display *disp, struct chip8 *chip8, const struct chip8_config *cfg) {
    (void) cfg;

    if (SDL_RenderClear(disp->renderer) < 0) {
        fprintf(stderr, "SDL_RenderClear(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
        chip8->display,
        DISPLAY_WIDTH, DISPLAY_HEIGHT, 1,
        DISPLAY_WIDTH / 8,
        SDL_PIXELFORMAT_INDEX1MSB
    );
    if (surface == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurfaceWithFormatFrom(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Color colors[] = {{0x20, 0x20, 0x20, 0xFF}, {0xF0, 0xF0, 0xF0, 0xFF}};
    if (SDL_SetPaletteColors(surface->format->palette, colors, 0, 2) < 0) {
        fprintf(stderr, "SDL_SetPaletteColors(): %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(disp->renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTextureFromSurface(): %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    if (SDL_RenderCopyF(disp->renderer, texture, NULL, NULL) < 0) {
        fprintf(stderr, "SDL_RenderCopyF(): %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        return -1;
    }

    SDL_RenderPresent(disp->renderer);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    return 0;
}

int chip8_run(struct chip8 *chip8, struct chip8_display *disp, const struct chip8_config *cfg) {
    (void) cfg;

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
                    case SDL_SCANCODE_0: chip8->keys[0x0] = true; break;
                    case SDL_SCANCODE_1: chip8->keys[0x1] = true; break;
                    case SDL_SCANCODE_2: chip8->keys[0x2] = true; break;
                    case SDL_SCANCODE_3: chip8->keys[0x3] = true; break;
                    case SDL_SCANCODE_4: chip8->keys[0x4] = true; break;
                    case SDL_SCANCODE_5: chip8->keys[0x5] = true; break;
                    case SDL_SCANCODE_6: chip8->keys[0x6] = true; break;
                    case SDL_SCANCODE_7: chip8->keys[0x7] = true; break;
                    case SDL_SCANCODE_8: chip8->keys[0x8] = true; break;
                    case SDL_SCANCODE_9: chip8->keys[0x9] = true; break;
                    case SDL_SCANCODE_A: chip8->keys[0xA] = true; break;
                    case SDL_SCANCODE_B: chip8->keys[0xB] = true; break;
                    case SDL_SCANCODE_C: chip8->keys[0xC] = true; break;
                    case SDL_SCANCODE_D: chip8->keys[0xD] = true; break;
                    case SDL_SCANCODE_E: chip8->keys[0xE] = true; break;
                    case SDL_SCANCODE_F: chip8->keys[0xF] = true; break;
                    case SDL_SCANCODE_P: pause = !pause; break;
                    case SDL_SCANCODE_ESCAPE: should_continue = false; break;
                    default: break;
                }
            }

            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_0: chip8->keys[0x0] = false; break;
                    case SDL_SCANCODE_1: chip8->keys[0x1] = false; break;
                    case SDL_SCANCODE_2: chip8->keys[0x2] = false; break;
                    case SDL_SCANCODE_3: chip8->keys[0x3] = false; break;
                    case SDL_SCANCODE_4: chip8->keys[0x4] = false; break;
                    case SDL_SCANCODE_5: chip8->keys[0x5] = false; break;
                    case SDL_SCANCODE_6: chip8->keys[0x6] = false; break;
                    case SDL_SCANCODE_7: chip8->keys[0x7] = false; break;
                    case SDL_SCANCODE_8: chip8->keys[0x8] = false; break;
                    case SDL_SCANCODE_9: chip8->keys[0x9] = false; break;
                    case SDL_SCANCODE_A: chip8->keys[0xA] = false; break;
                    case SDL_SCANCODE_B: chip8->keys[0xB] = false; break;
                    case SDL_SCANCODE_C: chip8->keys[0xC] = false; break;
                    case SDL_SCANCODE_D: chip8->keys[0xD] = false; break;
                    case SDL_SCANCODE_E: chip8->keys[0xE] = false; break;
                    case SDL_SCANCODE_F: chip8->keys[0xF] = false; break;
                    default: break;
                }
            }
            
            if (event.type == SDL_QUIT) {
                should_continue = false;
            }
        }

        if (!pause) {
            while (num_instructions < cfg->target_speed * ((frame_number % 60) + 1) / 60) {
                uint16_t instruction = chip8_get_instruction(chip8, cfg);
                chip8_exec(chip8, cfg, instruction);
                num_instructions++;
            }

            if (chip8->delay_timer > 0) {
                chip8->delay_timer--;
            }

            if (chip8->sound_timer > 0) {
                chip8->sound_timer--;
            }
        }

        if (chip8_draw(disp, chip8, cfg) == -1) {
            should_continue = false;
            return_value = -1;
            break;
        }

        frame_number++;
        num_instructions = 0;
        wait_until(next_time);
    }

    return return_value;
}
