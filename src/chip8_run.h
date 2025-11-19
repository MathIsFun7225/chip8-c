#ifndef CHIP8_RUN_H
#define CHIP8_RUN_H

#include "chip8_audio.h"
#include "chip8_config.h"
#include "chip8_display.h"
#include "chip8_state.h"

int chip8_advance(struct chip8_state *state, struct chip8_display *display, struct chip8_audio *audio, const struct chip8_config *config);
int chip8_rewind(struct chip8_state *state, struct chip8_display *display, struct chip8_audio *audio, const struct chip8_config *config);

#endif // CHIP8_RUN_H
