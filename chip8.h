#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define FONT_OFFSET 0
#define FONT_LENGTH 0x50

#define MAX_PROGRAM_LENGTH (sizeof(((struct chip8 *) NULL)->memory) - PROGRAM_OFFSET)
#define PROGRAM_OFFSET 0x200

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

struct chip8 {
    uint8_t memory[4096];

    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

    uint8_t registers[16];
    uint16_t index_register;
    uint8_t delay_timer;
    uint8_t sound_timer;

    uint16_t *stack;
    uint16_t stack_size;
    uint16_t sp;

    uint16_t pc;

    bool keys[16];
};

struct chip8_config {
    int target_speed;
    int default_scale;
};

struct chip8_display {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

int chip8_init(struct chip8 *chip8, const struct chip8_config *cfg);
int chip8_init_sdl(struct chip8_display *disp, const struct chip8_config *cfg);
int chip8_close(struct chip8 *chip8, const struct chip8_config *cfg);
int chip8_close_sdl(struct chip8_display *disp, const struct chip8_config *cfg);

int chip8_load_font_arr(struct chip8 *chip8, const struct chip8_config *cfg, const uint8_t *buf);
int chip8_load_font_file(struct chip8 *chip8, const struct chip8_config *cfg, const char *file);

int chip8_load_program_arr(struct chip8 *chip8, const struct chip8_config *cfg, const uint8_t *buf, uint16_t length);
int chip8_load_program_file(struct chip8 *chip8, const struct chip8_config *cfg, const char *file);

uint16_t chip8_get_instruction(const struct chip8 *chip8, const struct chip8_config *cfg);
int chip8_draw(struct chip8_display *disp, struct chip8 *chip8, const struct chip8_config *cfg);

int chip8_run(struct chip8 *chip8, struct chip8_display *disp, const struct chip8_config *cfg);

#include "chip8_exec.h"

#endif // CHIP8_H
