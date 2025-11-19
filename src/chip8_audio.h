#ifndef CHIP8_AUDIO_H
#define CHIP8_AUDIO_H

#include <SDL2/SDL.h>

struct chip8_audio {
    SDL_AudioDeviceID audio_device;
};

#include "chip8_config.h"
#include "chip8_state.h"

int chip8_init_audio(struct chip8_audio *audio, const struct chip8_config *config);
int chip8_close_audio(struct chip8_audio *audio, const struct chip8_config *config);

int chip8_play_audio(struct chip8_audio *audio, const struct chip8_config *config);
int chip8_pause_audio(struct chip8_audio *audio, const struct chip8_config *config);
int chip8_update_audio(struct chip8_audio *audio, const struct chip8_state *state, const struct chip8_config *config);

#endif // CHIP8_AUDIO_H
