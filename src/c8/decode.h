/**
 * @file c8/decode.h
 *
 * Stuff for disassembling bytecode.
 */

#ifndef C8_DECODE_H
#define C8_DECODE_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Auto-generate label names
 */
#define C8_DECODE_DEFINE_LABELS 0x1

/**
 * @brief Print addresses for each instruction
 */
#define C8_DECODE_PRINT_ADDRESSES 0x2

void     c8_decode(FILE*, FILE*, int);
char*    c8_decode_instruction(uint16_t, uint8_t*);
uint16_t c8_jump(uint16_t);

#endif
