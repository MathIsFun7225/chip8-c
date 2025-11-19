#ifndef CHIP8_EXEC_H
#define CHIP8_EXEC_H

#include "chip8_config.h"
#include "chip8_state.h"

// An opcode will have the structure of either 0xHXYN, 0xHXNN, or 0xHNNN

#define get_h(opcode)  (((opcode) & 0xF000) >> 12)
#define get_x(opcode)  (((opcode) & 0x0F00) >> 8)
#define get_y(opcode)  (((opcode) & 0x00F0) >> 4)
#define get_n(opcode)   ((opcode) & 0x000F)
#define get_nn(opcode)  ((opcode) & 0x00FF)
#define get_nnn(opcode) ((opcode) & 0x0FFF)

int chip8_exec(struct chip8_state *state, const struct chip8_config *config, uint16_t opcode);

#endif // CHIP8_EXEC_H
