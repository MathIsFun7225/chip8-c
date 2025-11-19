#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

#include "chip8_audio.h"
#include "chip8_config.h"
#include "chip8_display.h"
#include "chip8_state.h"

int chip8_load_program(struct chip8_state *state, const struct chip8_config *config, const char *file);
int chip8_run(struct chip8_state *state, struct chip8_display *display, struct chip8_audio *audio, const struct chip8_config *config);

#endif // CHIP8_H
