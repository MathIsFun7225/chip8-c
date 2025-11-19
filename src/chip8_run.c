#include "chip8_audio.h"
#include "chip8_config.h"
#include "chip8_display.h"
#include "chip8_run.h"
#include "chip8_state.h"

int chip8_advance(struct chip8_state *state, struct chip8_display *display, struct chip8_audio *audio, const struct chip8_config *config) {
    if (chip8_advance_state(state, config) == -1) {
        return -1;
    }

    if (chip8_update_display(display, state, config) == -1) {
        return -1;
    }

    if (chip8_update_audio(audio, state, config) == -1) {
        return -1;
    }

    return 0;
}

// int chip8_rewind(struct chip8_state *state, struct chip8_display *display, struct chip8_audio *audio, const struct chip8_config *config) {
//     if (chip8_rewind_state(state, config) == -1) {
//         return -1;
//     }

//     if (chip8_update_display(state, display, config) == -1) {
//         return -1;
//     }

//     if (chip8_update_audio(state, audio, config) == -1) {
//         return -1;
//     }

//     return 0;
// }
