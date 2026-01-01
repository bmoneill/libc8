/**
 * @file c8/private/instruction.h
 * @note NOT EXPORTED
 *
 * This file contains the declaration of the instruction parsing and
 * execution functions for the CHIP-8 interpreter.
 */

#ifndef LIBC8_INSTRUCTION_H
#define LIBC8_INSTRUCTION_H

#include "../chip8.h"

int c8_parse_instruction(C8* c8);

#endif
