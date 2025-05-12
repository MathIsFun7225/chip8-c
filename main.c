#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "chip8.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("Usage: chip8 <file.ch8>\n", stderr);
        return EXIT_FAILURE;
    }

    struct chip8 chip8;
    struct chip8_config cfg;
    struct chip8_display disp;

    cfg.target_speed = 500;
    cfg.default_scale = 10;

    if (chip8_init(&chip8, &cfg) == -1) {
        return EXIT_FAILURE;
    }

    if (chip8_load_program_file(&chip8, &cfg, argv[1]) == -1) {
        chip8_close(&chip8, &cfg);
        return EXIT_FAILURE;
    }

    if (chip8_init_sdl(&disp, &cfg) == -1) {
        chip8_close(&chip8, &cfg);
        return EXIT_FAILURE;
    }

    if (chip8_run(&chip8, &disp, &cfg) == -1) {
        chip8_close(&chip8, &cfg);
        chip8_close_sdl(&disp, &cfg);
        return EXIT_FAILURE;
    }

    chip8_close(&chip8, &cfg);
    chip8_close_sdl(&disp, &cfg);
    
    return 0;
}
