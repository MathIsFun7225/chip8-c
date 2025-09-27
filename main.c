#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "chip8.h"
#include "chip8_audio.h"
#include "chip8_config.h"
#include "chip8_display.h"
#include "chip8_exec.h"
#include "chip8_state.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("Usage: chip8 <file>\n", stderr);
        return EXIT_FAILURE;
    }

    struct chip8_state state;
    struct chip8_config config;
    struct chip8_display display;
    struct chip8_audio audio;

    config.target_speed = 500;
    config.default_scale = 10;

    if (chip8_init_state(&state, &config) == -1) {
        return EXIT_FAILURE;
    }

    if (chip8_load_program(&state, &config, argv[1]) == -1) {
        chip8_close_audio(&audio, &config);
        chip8_close_display(&display, &config);
        chip8_close_state(&state, &config);
        return EXIT_FAILURE;
    }

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init(): %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (chip8_init_display(&display, &config) == -1) {
        chip8_close_state(&state, &config);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    if (chip8_init_audio(&audio, &config) == -1) {
        chip8_close_display(&display, &config);
        chip8_close_state(&state, &config);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    if (chip8_run(&state, &display, &audio, &config) == -1) {
        chip8_close_audio(&audio, &config);
        chip8_close_display(&display, &config);
        chip8_close_state(&state, &config);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    chip8_close_audio(&audio, &config);
    chip8_close_display(&display, &config);
    SDL_Quit();
    chip8_close_state(&state, &config);
    
    return 0;
}
