#include <SDL2/SDL.h>

#include "chip8_config.h"
#include "chip8_display.h"
#include "chip8_state.h"

int chip8_init_display(struct chip8_display *display, const struct chip8_config *config) {
    display->window = SDL_CreateWindow(
        "chip8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH * config->default_scale, DISPLAY_HEIGHT * config->default_scale,
        SDL_WINDOW_RESIZABLE
    );
    if (display->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    if (display->renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer(): %s\n", SDL_GetError());
        SDL_DestroyWindow(display->window);
        display->window = NULL;
        return -1;
    }

    if (SDL_RenderSetLogicalSize(display->renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT) < 0) {
        fprintf(stderr, "SDL_RenderSetLogicalSize(): %s\n", SDL_GetError());
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        memset(display, 0, sizeof(*display));
        return -1;
    }

    if (SDL_RenderSetIntegerScale(display->renderer, SDL_TRUE) < 0) {
        fprintf(stderr, "SDL_RenderSetIntegerScale(): %s\n", SDL_GetError());
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        memset(display, 0, sizeof(*display));
        return -1;
    }

    return 0;
}

int chip8_close_display(struct chip8_display *display, const struct chip8_config *config) {
    (void) config;

    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    memset(display, 0, sizeof(*display));

    return 0;
}

int chip8_update_display(struct chip8_display *display, struct chip8_state *state, const struct chip8_config *config) {
    (void) config;

    if (SDL_RenderClear(display->renderer) < 0) {
        fprintf(stderr, "SDL_RenderClear(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
        state->screen,
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

    SDL_Texture *texture = SDL_CreateTextureFromSurface(display->renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTextureFromSurface(): %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    if (SDL_RenderCopyF(display->renderer, texture, NULL, NULL) < 0) {
        fprintf(stderr, "SDL_RenderCopyF(): %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        return -1;
    }

    SDL_RenderPresent(display->renderer);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    return 0;
}
