#ifndef CHIP8_EXEC_H
#define CHIP8_EXEC_H

#include <stdbool.h>
#include <stdint.h>

#include "chip8.h"

int chip8_exec(struct chip8 *chip8, const struct chip8_config *cfg, uint16_t instruction);
bool chip8_should_draw(uint16_t instruction);

#endif // CHIP8_EXEC_H
