#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8_config.h"
#include "chip8_exec.h"
#include "chip8_state.h"
#include "test.h"

enum chip8_compare {
    COMP_MEMORY = 1 << 0,
    COMP_SCREEN = 1 << 1,
    COMP_RES    = 1 << 2,
    COMP_REGS   = 1 << 3,
    COMP_INDEX  = 1 << 4,
    COMP_DELAY  = 1 << 5,
    COMP_SOUND  = 1 << 6,
    COMP_PC     = 1 << 7,
    COMP_RPL    = 1 << 8,
    COMP_STACK  = 1 << 9,
    COMP_ALL = COMP_MEMORY | COMP_SCREEN | COMP_RES | COMP_REGS | COMP_INDEX | COMP_DELAY | COMP_SOUND | COMP_PC | COMP_RPL | COMP_STACK
};

int chip8_compare_states(const struct chip8_state *s1, const struct chip8_state *s2, enum chip8_compare comp) {
    int result = 0;

    // Memory
    if (comp & COMP_MEMORY) {
        if (memcmp(s1->memory, s2->memory, sizeof(s1->memory)) != 0) {
            result |= COMP_MEMORY;
        }
    }

    // Screen
    if (comp & COMP_SCREEN) {
        if (memcmp(s1->screen, s2->screen, sizeof(s1->screen)) != 0) {
            result |= COMP_SCREEN;
        }
    }

    // Resolution
    if (comp & COMP_RES) {
        if (s1->hires != s2->hires) {
            result |= COMP_RES;
        }
    }

    // Registers
    if (comp & COMP_REGS) {
        if (memcmp(s1->registers, s2->registers, sizeof(s1->registers)) != 0) {
            result |= COMP_REGS;
        }
    }

    // Index
    if (comp & COMP_INDEX) {
        if (s1->index_register != s2->index_register) {
            result |= COMP_INDEX;
        }
    }

    // Delay
    if (comp & COMP_DELAY) {
        if (s1->delay_timer != s2->delay_timer) {
            result |= COMP_DELAY;
        }
    }

    // Sound
    if (comp & COMP_SOUND) {
        if (s1->sound_timer != s2->sound_timer) {
            result |= COMP_SOUND;
        }
    }
    
    // PC
    if (comp & COMP_PC) {
        if (s1->pc != s2->pc) {
            result |= COMP_PC;
        }
    }
    
    // RPL
    if (comp & COMP_RPL) {
        if (memcmp(s1->rpl_flags, s2->rpl_flags, sizeof(s1->rpl_flags)) != 0) {
            result |= COMP_RPL;
        }
    }
    
    // Stack
    if (comp & COMP_STACK) {
        if ((s1->sp != s2->sp) || (memcmp(s1->stack, s2->stack, s1->sp * sizeof(*s1->stack)) != 0)) {
            result |= COMP_STACK;
        }
    }

    return result;
}

void chip8_diff_states(const struct chip8_state *s1, const struct chip8_state *s2, enum chip8_compare comp) {
    // Memory
    if (comp & COMP_MEMORY) {
        uint16_t memory_diffs = 0;
        int first_memory_diff = -1;
        for (uint16_t i = 0; i < sizeof(s1->memory); i++) {
            if (s1->memory[i] != s2->memory[i]) {
                memory_diffs++;
                if (first_memory_diff == -1) {
                    first_memory_diff = i;
                }
            }
        }
        if (memory_diffs == 1) {
            printf("Memory: 1 diff at %04x: %02"PRIx8" != %02"PRIx8"\n",
                (unsigned) first_memory_diff, s1->memory[first_memory_diff], s2->memory[first_memory_diff]);
        } else if (memory_diffs > 0) {
            printf("Memory: %"PRIu16" diffs, first at %02"PRIx8": %02"PRIx8" != %02"PRIx8"\n",
                memory_diffs, first_memory_diff, s1->memory[first_memory_diff], s2->memory[first_memory_diff]);
        } else {
            puts("Memory: match");
        }
    }

    // Screen
    if (comp & COMP_SCREEN) {
        uint16_t screen_diffs = 0;
        int first_screen_diff = -1;
        for (uint16_t i = 0; i < sizeof(s1->screen); i++) {
            if (s1->screen[i] != s2->screen[i]) {
                screen_diffs++;
                if (first_screen_diff == -1) {
                    first_screen_diff = i;
                }
            }
        }
        if (screen_diffs == 1) {
            printf("Screen: 1 diff at %04u: %02"PRIx8" != %02"PRIx8"\n",
                (unsigned) first_screen_diff, s1->memory[first_screen_diff], s2->memory[first_screen_diff]);
        } else if (screen_diffs > 0) {
            printf("Screen: %"PRIu16" diffs, first at %02"PRIx8": %02"PRIx8" != %02"PRIx8"\n",
                screen_diffs, first_screen_diff, s1->screen[first_screen_diff], s2->memory[first_screen_diff]);
        } else {
            puts("Screen: match");
        }
    }

    // Resolution
    if (comp & COMP_RES) {
        if (s1->hires != s2->hires) {
            printf("   Res: %s != %s\n", s1->hires ? "hires" : "lores", s2->hires ? "hires" : "lores");
        } else {
            puts("   Res: match");
        }
    }

    // Registers
    if (comp & COMP_REGS) {
        uint8_t reg_diffs = 0;
        bool regs[16];
        for (uint8_t i = 0; i < 16; i++) {
            if ((regs[i] = (s1->registers[i] != s2->registers[i]))) {
                reg_diffs++;
            }
        }
        if (reg_diffs == 1) {
            fputs("  Regs: 1 diff: ", stdout);
            for (uint8_t i = 0; i < 16; i++) {
                if (regs[i]) {
                    printf("V%c %02"PRIx8" %02"PRIx8"\n", (i < 10) ? '0' + i : 'A' + (i - 10), s1->registers[i], s2->registers[i]);
                }
            }
        } else if (reg_diffs > 0) {
            uint8_t diffs_so_far = 0;
            printf("  Regs: %"PRIu8" diffs: ", reg_diffs);
            for (uint8_t i = 0; i < 16; i++) {
                if (regs[i]) {
                    diffs_so_far++;
                    char c = (i < 10) ? '0' + i : 'A' + (i - 10);
                    printf("V%c %02"PRIx8" %02"PRIx8"%s", c, s1->registers[i], s2->registers[i], (diffs_so_far == reg_diffs) ? "" : ", ");
                }
            }
            putchar('\n');
        } else {
            puts("  Regs: match");
        }
    }

    // Index
    if (comp & COMP_INDEX) {
        if (s1->index_register != s2->index_register) {
            printf(" Index: %04"PRIx16" != %04"PRIx16"\n", s1->index_register, s2->index_register);
        } else {
            puts(" Index: match");
        }
    }

    // Delay
    if (comp & COMP_DELAY) {
        if (s1->delay_timer != s2->delay_timer) {
            printf(" Delay: %02"PRIx8" != %02"PRIx8"\n", s1->delay_timer, s2->delay_timer);
        } else {
            puts(" Delay: match");
        }
    }

    // Sound
    if (comp & COMP_SOUND) {
        if (s1->sound_timer != s2->sound_timer) {
            printf(" Sound: %02"PRIx8" != %02"PRIx8"\n", s1->sound_timer, s2->sound_timer);
        } else {
            puts(" Sound: match");
        }
    }

    // PC
    if (comp & COMP_PC) {
        if (s1->pc != s2->pc) {
            printf("    PC: %04"PRIx16" != %04"PRIx16"\n", s1->pc, s2->pc);
        } else {
            puts("    PC: match");
        }
    }

    // RPL
    if (comp & COMP_RPL) {
        uint8_t rpl_diffs = 0;
        bool rpl[8];
        for (uint8_t i = 0; i < 8; i++) {
            if ((rpl[i] = (s1->rpl_flags[i] != s2->rpl_flags[i]))) {
                rpl_diffs++;
            }
        }
        if (rpl_diffs == 1) {
            fputs("   RPL: 1 diff: ", stdout);
            for (uint8_t i = 0; i < 8; i++) {
                if (rpl[i]) {
                    printf("%"PRIu8" %02"PRIx8" %02"PRIx8"\n", i, s1->rpl_flags[i], s2->rpl_flags[i]);
                }
            }
        } else if (rpl_diffs > 0) {
            uint8_t diffs_so_far = 0;
            printf("   RPL: %"PRIu8" diffs: ", rpl_diffs);
            for (uint8_t i = 0; i < 8; i++) {
                if (rpl[i]) {
                    diffs_so_far++;
                    printf("%"PRIu8" %02"PRIx8" %02"PRIx8"%s", i, s1->rpl_flags[i], s2->rpl_flags[i], (diffs_so_far == rpl_diffs) ? "" : ", ");
                }
            }
            putchar('\n');
        } else {
            puts("   RPL: match");
        }
    }

    // Stack
    if (comp & COMP_STACK) {
        bool sp_match = (s1->sp == s2->sp);
        bool content_match = sp_match;
        if (sp_match) {
            for (uint16_t i = s1->sp; i > 0; i--) {
                if (s1->stack[i] != s2->stack[i]) {
                    content_match = false;
                    break;
                }
            }
        }
        if (!sp_match) {
            printf(" Stack: sp %04"PRIx16" %04"PRIx16"\n", s1->sp, s2->sp);
        } else {
            puts(" Stack: sp match");
        }
        printf("        ");
        if (!content_match) {
            if (s1->sp > 0) {
                for (uint16_t i = s1->sp; i > 0; i--) {
                    printf("%04"PRIx16"%s", s1->stack[i - 1], (i > 1 ? " " : ""));
                }
            } else {
                fputs("(empty)", stdout);
            }
            fputs("\n        ", stdout);
            
            if (s2->sp > 0) {
                for (uint16_t i = s2->sp; i > 0; i--) {
                    printf("%04"PRIx16"%s", s2->stack[i - 1], (i > 1 ? " " : ""));
                }
            } else {
                fputs("(empty)", stdout);
            }
            putchar('\n');
        } else {
            puts("content match");
        }
    }
}

void test_scroll(void) {
    struct chip8_state initial, expected;

    // 00B0 - Empty screen, up by 0
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xB0}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xB0}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00B1 - Empty screen, up by 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xB1}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xB1}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00C0 - Empty screen, down by 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xC0}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xC0}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00C1 - Empty screen, down by 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xC1}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xC1}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FB - Empty screen, right by 4
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFB}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFB}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FC - Empty screen, left by 4
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFC}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFC}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);


    const uint8_t nonempty_screen[] = {[0] = 0xFF, [15] = 0xFF, [1008] = 0xFF, [1023] = 0xFF};

    // 00B0 - Nonempty screen, up by 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xB0}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xB0}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00C0 - Nonempty screen, down by 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xC0}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xC0}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00B1 - Nonempty screen, up by 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xB1}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.screen, (const uint8_t []){[992] = 0xFF, [1007] = 0xFF}, 1008);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xB1}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00C1 - Nonempty screen, down by 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xC1}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.screen, (const uint8_t []){[16] = 0xFF, [31] = 0xFF}, 32);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xC1}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FB - Nonempty screen, right by 4
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFB}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.screen, (const uint8_t []){[0] = 0x0F, [1] = 0xF0, [15] = 0x0F, [1008] = 0x0F, [1009] = 0xF0, [1023] = 0x0F}, sizeof(expected.screen));
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFB}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FC - Nonempty screen, left by 4
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.screen, nonempty_screen, sizeof(nonempty_screen));
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFC}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.screen, (const uint8_t []){[0] = 0xF0, [14] = 0x0F, [15] = 0xF0, [1008] = 0xF0, [1022] = 0x0F, [1023] = 0xF0}, sizeof(expected.screen));
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFC}, 2);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_draw(void) {
    struct chip8_state initial, expected;

    // D000 - draw 8x0 sprite in lores at (0, 0) on empty screen
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x00}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x00}, 2);
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // D001 - draw empty 8x1 sprite in lores at (0, 0) on empty screen
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x01}, 2);
    initial.index_register = initial.pc + 2;
    initial.memory[initial.index_register] = 0x00;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x01}, 2);
    expected.index_register = expected.pc + 2;
    expected.memory[expected.index_register] = 0x00;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // D001 - draw nonempty 8x1 sprite in lores at (0, 0) on empty screen
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x01}, 2);
    initial.index_register = initial.pc + 2;
    initial.memory[initial.index_register] = 0xFF;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x01}, 2);
    expected.index_register = expected.pc + 2;
    expected.memory[expected.index_register] = 0xFF;
    expected.screen[0] = 0xFF;
    expected.screen[1] = 0xFF;
    expected.screen[16] = 0xFF;
    expected.screen[17] = 0xFF;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // D012 - draw nonempty 8x2 sprite in lores at (4, 1) on empty screen
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x12}, 2);
    initial.registers[0x0] = 4;
    initial.registers[0x1] = 1;
    initial.index_register = initial.pc + 2;
    initial.memory[initial.index_register] = 0xFF;
    initial.memory[initial.index_register + 1] = 0x0F;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x12}, 2);
    expected.registers[0x0] = 4;
    expected.registers[0x1] = 1;
    expected.index_register = expected.pc + 2;
    expected.memory[expected.index_register] = 0xFF;
    expected.memory[expected.index_register + 1] = 0x0F;
    expected.screen[33] = 0xFF;
    expected.screen[34] = 0xFF;
    expected.screen[49] = 0xFF;
    expected.screen[50] = 0xFF;
    expected.screen[66] = 0xFF;
    expected.screen[82] = 0xFF;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // D011 - draw nonempty 8x1 sprite in hires at (128, 64) == (0, 0) on empty screen
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x11}, 2);
    initial.hires = true;
    initial.index_register = initial.pc + 2;
    initial.memory[initial.index_register] = 0xFF;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x11}, 2);
    expected.hires = true;
    expected.index_register = expected.pc + 2;
    expected.memory[expected.index_register] = 0xFF;
    expected.screen[0] = 0xFF;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // D010 - draw nonempty 16x16 sprite in hires at (4, 1) on empty screen
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x10}, 2);
    initial.hires = true;
    initial.registers[0x0] = 4;
    initial.registers[0x1] = 1;
    initial.index_register = initial.pc + 2;
    memcpy(&initial.memory[initial.index_register], (const uint8_t []){
        0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F,
        0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F
    }, 32);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x10}, 2);
    expected.hires = true;
    expected.registers[0x0] = 4;
    expected.registers[0x1] = 1;
    expected.index_register = expected.pc + 2;
    memcpy(&expected.memory[expected.index_register], (const uint8_t []){
        0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F,
        0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F
    }, 32);
    for (int i = 0; i < 16; i++) {
        expected.screen[16 * (i + 1)] = 0x0F;
        expected.screen[16 * (i + 1) + 1] = 0xF0;
        expected.screen[16 * (i + 1) + 2] = 0xF0;
    }
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq_mem(initial.screen, expected.screen, sizeof(initial.screen));
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // D012 - draw nonempty 8x2 sprite in hires at (124, 63) on nonempty screen
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xD0, 0x12}, 2);
    initial.hires = true;
    initial.registers[0x0] = 124;
    initial.registers[0x1] = 63;
    initial.index_register = initial.pc + 2;
    initial.memory[initial.index_register] = 0xFF;
    initial.memory[initial.index_register + 1] = 0xFF;
    initial.screen[0] = 0xFF;
    initial.screen[15] = 0xFF;
    initial.screen[1008] = 0xFF;
    initial.screen[1023] = 0xFF;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xD0, 0x12}, 2);
    expected.hires = true;
    expected.registers[0x0] = 124;
    expected.registers[0x1] = 63;
    expected.index_register = expected.pc + 2;
    expected.memory[expected.index_register] = 0xFF;
    expected.memory[expected.index_register + 1] = 0xFF;
    expected.screen[0] = 0xFF;
    expected.screen[15] = 0xFF;
    expected.screen[1008] = 0xFF;
    expected.screen[1023] = 0xF0;
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq_mem(initial.screen, expected.screen, sizeof(initial.screen));
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_func(void) {
    struct chip8_state initial, expected;

    // 2800 - call 0x800
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x28, 0x00}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x28, 0x00}, 2);
    expected.pc = 0x800;
    expected.stack[0] = 0x202;
    expected.sp = 1;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00EE - return to 0x202
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xEE}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xEE}, 2);
    expected.pc = 0x202;
    expected.sp = 0;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 2CCC - call 0xCCC from non-starting pc
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    initial.pc = 0x500;
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x2C, 0xCC}, 2);
    chip8_advance_state(&initial, NULL);
    expected.pc = 0x500;
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x2C, 0xCC}, 2);
    expected.pc = 0xCCC;
    expected.stack[0] = 0x502;
    expected.sp = 1;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00EE - return to 0x502
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xEE}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xEE}, 2);
    expected.pc = 0x502;
    expected.sp = 0;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_exit(void) {
    // 00FD
}

void test_res(void) {
    struct chip8_state initial, expected;

    // 00FF - enable hires when disabled
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFF}, 2);
    initial.hires = false;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFF}, 2);
    expected.pc += 2;
    expected.hires = true;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FF - enable hires when enabled
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFF}, 2);
    initial.hires = true;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFF}, 2);
    expected.pc += 2;
    expected.hires = true;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FE - disable hires when disabled
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFE}, 2);
    initial.hires = false;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFE}, 2);
    expected.hires = false;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 00FE - disable hires when enabled
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x00, 0xFE}, 2);
    initial.hires = true;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x00, 0xFE}, 2);
    expected.hires = false;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_jump(void) {
    struct chip8_state initial, expected;

    // 1567 - jump to 0x567
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x15, 0x67}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x15, 0x67}, 2);
    expected.pc = 0x567;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // B321 - jump to V0 (0x20) + 0x321
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xB3, 0x21}, 2);
    initial.registers[0] = 0x20;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xB3, 0x21}, 2);
    expected.registers[0] = 0x20;
    expected.pc = 0x341;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_skip(void) {
    struct chip8_state initial, expected;

    // 3208 - skip if V2 == 0x08 (false)
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x32, 0x08}, 2);
    initial.registers[0x2] = 0x9A;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x32, 0x08}, 2);
    expected.registers[0x2] = 0x9A;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 3763 - skip if V7 == 0x63 (true)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x37, 0x63}, 2);
    initial.registers[0x7] = 0x63;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x37, 0x63}, 2);
    expected.registers[0x7] = 0x63;
    expected.pc += 4;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 401C - skip if V0 != 0x1C (false)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x40, 0x1C}, 2);
    initial.registers[0x0] = 0x1C;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x40, 0x1C}, 2);
    expected.registers[0x0] = 0x1C;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 4E9C - skip if VE != 0x9C (true)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x4E, 0x9C}, 2);
    initial.registers[0xE] = 0x5B;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x4E, 0x9C}, 2);
    expected.registers[0xE] = 0x5B;
    expected.pc += 4;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);
    
    // 5BE0 - skip if VB == VE (false)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x5B, 0xE0}, 2);
    initial.registers[0xB] = 0xC0;
    initial.registers[0xE] = 0x95;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x5B, 0xE0}, 2);
    expected.registers[0xB] = 0xC0;
    expected.registers[0xE] = 0x95;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 5940 - skip if V9 == V4 (true)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x59, 0x40}, 2);
    initial.registers[0x9] = 0x2A;
    initial.registers[0x4] = 0x2A;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x59, 0x40}, 2);
    expected.registers[0x9] = 0x2A;
    expected.registers[0x4] = 0x2A;
    expected.pc += 4;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);
    
    // 9240 - skip if V2 != V4 (false)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x92, 0x40}, 2);
    initial.registers[0x2] = 0x25;
    initial.registers[0x4] = 0x25;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x92, 0x40}, 2);
    expected.registers[0x2] = 0x25;
    expected.registers[0x4] = 0x25;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 9930 - skip if V9 != V3 (true)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x99, 0x30}, 2);
    initial.registers[0x9] = 0x0A;
    initial.registers[0x3] = 0xAC;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x99, 0x30}, 2);
    expected.registers[0x9] = 0x0A;
    expected.registers[0x3] = 0xAC;
    expected.pc += 4;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // EB9E - skip if key in VB is pressed (false)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xEB, 0x9E}, 2);
    initial.registers[0xB] = 0xE;
    initial.keys[0xE] = false;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xEB, 0x9E}, 2);
    expected.registers[0xB] = 0xE;
    expected.keys[0xE] = false;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);
    
    // EE9E - skip if key in VE is pressed (true)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xEE, 0x9E}, 2);
    initial.registers[0xE] = 0xD;
    initial.keys[0xD] = true;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xEE, 0x9E}, 2);
    expected.registers[0xE] = 0xD;
    expected.keys[0xD] = true;
    expected.pc += 4;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);
    
    // EFA1 - skip if key in VF is not pressed (false)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xEF, 0xA1}, 2);
    initial.registers[0xF] = 0x1;
    initial.keys[0x1] = true;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xEF, 0xA1}, 2);
    expected.registers[0xF] = 0x1;
    expected.keys[0x1] = true;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // E4A1 - skip if key in V4 is not pressed (true)
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xE4, 0xA1}, 2);
    initial.registers[0x4] = 0x0;
    initial.keys[0x0] = false;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xE4, 0xA1}, 2);
    expected.registers[0x4] = 0x0;
    expected.keys[0x0] = false;
    expected.pc += 4;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_reg_ops(void) {
    struct chip8_state initial, expected;

    // 64BE - set V4 = 0xBE
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x64, 0xBE}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x64, 0xBE}, 2);
    expected.registers[0x4] = 0xBE;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 7C66 - set VC += 0x66, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x7C, 0x66}, 2);
    initial.registers[0xC] = 0x48;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x7C, 0x66}, 2);
    expected.registers[0xC] = 0xAE;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 7F70 - set VF += 0x70, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x7F, 0x70}, 2);
    initial.registers[0xF] = 0x61;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x7F, 0x70}, 2);
    expected.registers[0xF] = 0xD1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 7D41 - set VD += 0x41, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x7D, 0x41}, 2);
    initial.registers[0xD] = 0xD8;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x7D, 0x41}, 2);
    expected.registers[0xD] = 0x19;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 7F93 - set VF += 0x93, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x7F, 0x93}, 2);
    initial.registers[0xF] = 0xB3;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x7F, 0x93}, 2);
    expected.registers[0xF] = 0x46;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8B00 - set VB = V0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8B, 0x00}, 2);
    initial.registers[0x0] = 0xD0;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8B, 0x00}, 2);
    expected.registers[0x0] = 0xD0;
    expected.registers[0xB] = 0xD0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8591 - set V5 |= V9
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x85, 0x91}, 2);
    initial.registers[0x5] = 0x2C;
    initial.registers[0x9] = 0xCC;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x85, 0x91}, 2);
    expected.registers[0x5] = 0xEC;
    expected.registers[0x9] = 0xCC;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 82E2 - set V2 &= VE
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x82, 0xE2}, 2);
    initial.registers[0x2] = 0x60;
    initial.registers[0xE] = 0x8D;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x82, 0xE2}, 2);
    expected.registers[0x2] = 0x00;
    expected.registers[0xE] = 0x8D;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8AE3 - set VA ^= VE
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8A, 0xE3}, 2);
    initial.registers[0xA] = 0xC2;
    initial.registers[0xE] = 0x69;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8A, 0xE3}, 2);
    expected.registers[0xA] = 0xAB;
    expected.registers[0xE] = 0x69;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8334 - set V3 += V3, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x83, 0x34}, 2);
    initial.registers[0x3] = 0x31;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x83, 0x34}, 2);
    expected.registers[0x3] = 0x62;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F54 - set VF += V8, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x54}, 2);
    initial.registers[0xF] = 0xED;
    initial.registers[0x8] = 0x0D;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x54}, 2);
    expected.registers[0xF] = 0;
    expected.registers[0x8] = 0x0D;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8064 - set V0 += V6, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x80, 0x64}, 2);
    initial.registers[0x0] = 0xC7;
    initial.registers[0x6] = 0xEF;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x80, 0x64}, 2);
    expected.registers[0x0] = 0xB6;
    expected.registers[0x6] = 0xEF;
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F54 - set VF += V5, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x54}, 2);
    initial.registers[0xF] = 0x18;
    initial.registers[0x5] = 0xFE;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x54}, 2);
    expected.registers[0xF] = 1;
    expected.registers[0x5] = 0xFE;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8CD5 - set VC -= VD, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8C, 0xD5}, 2);
    initial.registers[0xC] = 0x14;
    initial.registers[0xD] = 0x02;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8C, 0xD5}, 2);
    expected.registers[0xC] = 0x12;
    expected.registers[0xD] = 0x02;
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8FE5 - set VF -= VE, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0xE5}, 2);
    initial.registers[0xF] = 0xF8;
    initial.registers[0xE] = 0x67;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0xE5}, 2);
    expected.registers[0xF] = 1;
    expected.registers[0xE] = 0x67;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8D35 - set VD -= V3, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8D, 0x35}, 2);
    initial.registers[0xD] = 0xDC;
    initial.registers[0x3] = 0xED;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8D, 0x35}, 2);
    expected.registers[0xD] = 0xEF;
    expected.registers[0x3] = 0xED;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F95 - set VF -= V9, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x95}, 2);
    initial.registers[0xF] = 0xC1;
    initial.registers[0x9] = 0xFF;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x95}, 2);
    expected.registers[0xF] = 0;
    expected.registers[0x9] = 0xFF;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8406 - set V4 >>= 1, shift 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x84, 0x06}, 2);
    initial.registers[0x4] = 0x94;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x84, 0x06}, 2);
    expected.registers[0x4] = 0x4A;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F06 - set VF >>= 1, shift 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x06}, 2);
    initial.registers[0xF] = 0x88;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x06}, 2);
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8E06 - set VE >>= 1, shift 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8E, 0x06}, 2);
    initial.registers[0xE] = 0xCD;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8E, 0x06}, 2);
    expected.registers[0xE] = 0x66;
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F06 - set VF >>= 1, shift 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x06}, 2);
    initial.registers[0xF] = 0xF5;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x06}, 2);
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8517 - set V5 = V1 - V5, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x85, 0x17}, 2);
    initial.registers[0x5] = 0x72;
    initial.registers[0x1] = 0xB1;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x85, 0x17}, 2);
    expected.registers[0x5] = 0x3F;
    expected.registers[0x1] = 0xB1;
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8FA7 - set VF = VA - VF, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0xA7}, 2);
    initial.registers[0xF] = 0x79;
    initial.registers[0xA] = 0xA7;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0xA7}, 2);
    expected.registers[0xF] = 1;
    expected.registers[0xA] = 0xA7;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8567 - set V5 = V6 - V5, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x85, 0x67}, 2);
    initial.registers[0x5] = 0x81;
    initial.registers[0x6] = 0x12;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x85, 0x67}, 2);
    expected.registers[0x5] = 0x91;
    expected.registers[0x6] = 0x12;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8FE7 - set VF = VE - VF, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0xE7}, 2);
    initial.registers[0xF] = 0x63;
    initial.registers[0xE] = 0x47;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0xE7}, 2);
    expected.registers[0xF] = 0;
    expected.registers[0xE] = 0x47;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 860E - set V6 <<= 1, shift 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x86, 0x0E}, 2);
    initial.registers[0x6] = 0x59;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x86, 0x0E}, 2);
    expected.registers[0x6] = 0xB2;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F0E - set VF <<= 1, shift 0
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x0E}, 2);
    initial.registers[0xF] = 0x33;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x0E}, 2);
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 850E - set V5 <<= 1, shift 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x85, 0x0E}, 2);
    initial.registers[0x5] = 0x90;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x85, 0x0E}, 2);
    expected.registers[0x5] = 0x20;
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // 8F0E - set VF <<= 1, shift 1
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0x8F, 0x0E}, 2);
    initial.registers[0xF] = 0xB0;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0x8F, 0x0E}, 2);
    expected.registers[0xF] = 1;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // AFDB - set I = 0xFDB
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xAF, 0xDB}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xAF, 0xDB}, 2);
    expected.index_register = 0xFDB;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // CA40 - set VA = 0x40 & rand()
    for (int i = 0; i < 10; i++) {
        chip8_reset_state(&initial, NULL);
        chip8_reset_state(&expected, NULL);
        memcpy(&initial.memory[initial.pc], (const uint8_t []){0xCA, 0x40}, 2);
        chip8_advance_state(&initial, NULL);
        memcpy(&expected.memory[expected.pc], (const uint8_t []){0xCA, 0x40}, 2);
        expected.pc += 2;
        expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL ^ COMP_REGS), 0);
        for (uint8_t j = 0; j < 0xA; j++) {
            expect_eq(initial.registers[j], expected.registers[j]);
        }
        expect_eq(initial.registers[0xA] | 0x40, 0x40);
        for (uint8_t j = 0xA + 1; j < 0x10; j++) {
            expect_eq(initial.registers[j], expected.registers[j]);
        }
    }

    // F607 - set V6 = delay
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF6, 0x07}, 2);
    initial.delay_timer = 0x98;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF6, 0x07}, 2);
    expected.delay_timer = 0x98;
    expected.registers[0x6] = 0x98;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F515 - set delay = V5
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF5, 0x15}, 2);
    initial.registers[0x5] = 0x1F;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF5, 0x15}, 2);
    expected.registers[0x5] = 0x1F;
    expected.delay_timer = 0x1F;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // FF18 - set sound = VF
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xFF, 0x18}, 2);
    initial.registers[0xF] = 0x9B;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xFF, 0x18}, 2);
    expected.registers[0xF] = 0x9B;
    expected.sound_timer = 0x9B;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // FA1E - set I += VA, no overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xFA, 0x1E}, 2);
    initial.index_register = 0x4BD;
    initial.registers[0xA] = 0xC5;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xFA, 0x1E}, 2);
    expected.index_register = 0x582;
    expected.registers[0xA] = 0xC5;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F91E - set I += V9, overflow
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF9, 0x1E}, 2);
    initial.index_register = 0xFB6;
    initial.registers[0x9] = 0x0BA;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF9, 0x1E}, 2);
    expected.index_register = 0x070;
    expected.registers[0x9] = 0xBA;
    expected.registers[0xF] = 0;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_key(void) {
    struct chip8_state initial, expected;

    // F80A - set V8 = key(), none pressed
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF8, 0x0A}, 2);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF8, 0x0A}, 2);
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    initial.keys[0xC] = true;
    chip8_advance_state(&initial, NULL);
    expected.keys[0xC] = true;
    expected.registers[0x8] = 0xC;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F10A - set V1 = key(), one pressed
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF1, 0x0A}, 2);
    initial.keys[0x5] = true;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF1, 0x0A}, 2);
    expected.keys[0x5] = true;
    expected.registers[0x1] = 0x5;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F40A - set V4 = key(), multiple pressed
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF4, 0x0A}, 2);
    initial.keys[0x3] = true;
    initial.keys[0x2] = true;
    initial.keys[0xF] = true;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF4, 0x0A}, 2);
    expected.keys[0x3] = true;
    expected.keys[0x2] = true;
    expected.keys[0xF] = true;
    expected.registers[0x4] = 0x2;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_bcd(void) {
    struct chip8_state initial, expected;

    // F133 - write BCD(V1) to memory at I
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF1, 0x33}, 2);
    initial.index_register = 0x110;
    initial.registers[0x1] = 0x9A;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF1, 0x33}, 2);
    expected.index_register = 0x110;
    expected.registers[0x1] = 0x9A;
    memcpy(&expected.memory[0x110], (const uint8_t []){1, 5, 4}, 3);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_sprite(void) {
    struct chip8_state initial, expected;

    // F229 - set I to location of lores (5-byte) sprite for V2
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF2, 0x29}, 2);
    initial.registers[0x2] = 0x1;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF2, 0x29}, 2);
    expected.registers[0x2] = 0x1;
    expected.index_register = 5;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F330 - set I to location of hires (10-byte) sprite for V3
    chip8_reset_state(&initial, NULL);
    chip8_reset_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF3, 0x30}, 2);
    initial.registers[0x3] = 0xF;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF3, 0x30}, 2);
    expected.registers[0x3] = 0xF;
    expected.index_register = 230;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

void test_reg_ldst(void) {
    struct chip8_state initial, expected;

    // F055 - write register V0 to memory at I
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF0, 0x55}, 2);
    initial.index_register = 0xDF9;
    initial.registers[0x0] = 0x1D;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF0, 0x55}, 2);
    expected.index_register = 0xDF9;
    expected.registers[0x0] = 0x1D;
    expected.memory[0xDF9] = 0x1D;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // FC55 - write registers V0-VC to memory at I
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xFC, 0x55}, 2);
    initial.index_register = 0x2CD;
    memcpy(initial.registers, (const uint8_t []){0x34, 0xAB, 0x6B, 0x13, 0x03, 0x8A, 0x46, 0x32, 0xF2, 0x38, 0x5F, 0x59, 0x80}, 13);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xFC, 0x55}, 2);
    expected.index_register = 0x2CD;
    memcpy(expected.registers, (const uint8_t []){0x34, 0xAB, 0x6B, 0x13, 0x03, 0x8A, 0x46, 0x32, 0xF2, 0x38, 0x5F, 0x59, 0x80}, 13);
    memcpy(&expected.memory[0x2CD], (const uint8_t []){0x34, 0xAB, 0x6B, 0x13, 0x03, 0x8A, 0x46, 0x32, 0xF2, 0x38, 0x5F, 0x59, 0x80}, 13);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F065 - read register V0 from memory at I
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF0, 0x65}, 2);
    initial.index_register = 0xA3F;
    initial.memory[0xA3F] = 0x43;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF0, 0x65}, 2);
    expected.index_register = 0xA3F;
    expected.memory[0xA3F] = 0x43;
    expected.registers[0x0] = 0x43;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // FA65 - read registers V0-VA from memory at I
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xFA, 0x65}, 2);
    initial.index_register = 0x39E;
    memcpy(&initial.memory[0x39E], (const uint8_t []){0xEF, 0xF1, 0xF7, 0x99, 0x0E, 0xB0, 0x07, 0x74, 0x2F, 0x56, 0x24}, 11);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xFA, 0x65}, 2);
    expected.index_register = 0x39E;
    memcpy(&expected.memory[0x39E], (const uint8_t []){0xEF, 0xF1, 0xF7, 0x99, 0x0E, 0xB0, 0x07, 0x74, 0x2F, 0x56, 0x24}, 11);
    memcpy(expected.registers, (const uint8_t []){0xEF, 0xF1, 0xF7, 0x99, 0x0E, 0xB0, 0x07, 0x74, 0x2F, 0x56, 0x24}, 11);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F075 - write register V0 to RPL flags
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF0, 0x75}, 2);
    initial.index_register = 0xF40;
    initial.registers[0x0] = 0x45;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF0, 0x75}, 2);
    expected.index_register = 0xF40;
    expected.registers[0x0] = 0x45;
    expected.rpl_flags[0x0] = 0x45;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F875 - write registers V0-V8 to RPL flags
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF8, 0x75}, 2);
    initial.index_register = 0xA46;
    memcpy(initial.registers, (const uint8_t []){0x95, 0x44, 0x5E, 0xE9, 0xCC, 0x4E, 0x59, 0x6C, 0x17}, 9);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF8, 0x75}, 2);
    expected.index_register = 0xA46;
    memcpy(expected.registers, (const uint8_t []){0x95, 0x44, 0x5E, 0xE9, 0xCC, 0x4E, 0x59, 0x6C, 0x17}, 9);
    memcpy(expected.rpl_flags, (const uint8_t []){0x95, 0x44, 0x5E, 0xE9, 0xCC, 0x4E, 0x59, 0x6C, 0x17}, 9);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // F085 - read register V0 from RPL flags
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xF0, 0x85}, 2);
    initial.index_register = 0xCF9;
    initial.rpl_flags[0x0] = 0xD5;
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xF0, 0x85}, 2);
    expected.index_register = 0xCF9;
    expected.rpl_flags[0x0] = 0xD5;
    expected.registers[0x0] = 0xD5;
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    // FD85 - read registers V0-VD from RPL flags
    chip8_init_state(&initial, NULL);
    chip8_init_state(&expected, NULL);
    memcpy(&initial.memory[initial.pc], (const uint8_t []){0xFD, 0x85}, 2);
    initial.index_register = 0x7A0;
    memcpy(initial.rpl_flags, (const uint8_t []){0x41, 0x6B, 0x1C, 0xF9, 0x27, 0x79, 0x53, 0x22, 0x5F, 0x2E, 0x1E, 0x04, 0xB2, 0x0B}, 14);
    chip8_advance_state(&initial, NULL);
    memcpy(&expected.memory[expected.pc], (const uint8_t []){0xFD, 0x85}, 2);
    expected.index_register = 0x7A0;
    memcpy(expected.rpl_flags, (const uint8_t []){0x41, 0x6B, 0x1C, 0xF9, 0x27, 0x79, 0x53, 0x22, 0x5F, 0x2E, 0x1E, 0x04, 0xB2, 0x0B}, 14);
    memcpy(expected.registers, (const uint8_t []){0x41, 0x6B, 0x1C, 0xF9, 0x27, 0x79, 0x53, 0x22, 0x5F, 0x2E, 0x1E, 0x04, 0xB2, 0x0B}, 14);
    expected.pc += 2;
    expect_eq(chip8_compare_states(&initial, &expected, COMP_ALL), 0);

    chip8_close_state(&initial, NULL);
    chip8_close_state(&expected, NULL);
}

int main(void) {
    test_scroll();
    test_draw();
    test_func();
    test_res();
    test_jump();
    test_skip();
    test_reg_ops();
    test_key();
    test_bcd();
    test_sprite();
    test_reg_ldst();

    summarize_tests();
    return EXIT_SUCCESS;
}
