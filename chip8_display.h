#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include <SDL2/SDL.h>

struct chip8_display {
    SDL_Renderer *renderer;
    SDL_Window *window;
};

#include "chip8_config.h"
#include "chip8_state.h"

int chip8_init_display(struct chip8_display *display, const struct chip8_config *config);
int chip8_close_display(struct chip8_display *display, const struct chip8_config *config);
int chip8_update_display(struct chip8_display *display, struct chip8_state *state, const struct chip8_config *config);

#endif // CHIP8_DISPLAY_H
