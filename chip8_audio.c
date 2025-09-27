#include <math.h>
#include <SDL2/SDL.h>

#include "chip8_audio.h"
#include "chip8_config.h"
#include "chip8_state.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846L
#endif

static const int SAMPLE_RATE = 44100;
static const int NUM_SAMPLES = 4096;
static const double VOLUME = 0.8;

static int g_step_num = 0;
static int g_frequency;
static double g_step_size;

static void SDLCALL audio_callback(void *userdata, uint8_t *stream, int len) {
    (void) userdata;
    
    float *fstream = (float *) stream;
    for (int i = 0; i < len / sizeof(float); i++) {
        double value = VOLUME * sin(g_step_num * g_step_size);
        fstream[i] = (float) value;
        g_step_num++;
    }
}

int chip8_init_audio(struct chip8_audio *audio, const struct chip8_config *config) {
    (void) config;

    g_frequency = 500;
    g_step_size = 2.0 * M_PI * (double) g_frequency / (double) SAMPLE_RATE;

    SDL_AudioSpec spec = {
        .freq = SAMPLE_RATE,
        .format = AUDIO_F32,
        .channels = 1,
        .samples = NUM_SAMPLES,
        .callback = audio_callback,
    };

    audio->audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audio->audio_device == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice(): %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

int chip8_close_audio(struct chip8_audio *audio, const struct chip8_config *config) {
    (void) config;

    SDL_CloseAudioDevice(audio->audio_device);
    return 0;
}

int chip8_play_audio(struct chip8_audio *audio, const struct chip8_config *config) {
    (void) config;

    SDL_PauseAudioDevice(audio->audio_device, 0);
    return 0;
}

int chip8_pause_audio(struct chip8_audio *audio, const struct chip8_config *config) {
    (void) config;

    SDL_PauseAudioDevice(audio->audio_device, 1);
    return 0;
}

int chip8_update_audio(struct chip8_audio *audio, const struct chip8_state *state, const struct chip8_config *config) {
    if (state->sound_timer > 0) {
        return chip8_play_audio(audio, config);
    } else {
        return chip8_pause_audio(audio, config);
    }
}
