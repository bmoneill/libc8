/**
 * @file c8/chip8.h
 *
 * Stuff for simulating and modifying `CHIP8`s.
 */

#ifndef C8_CHIP8_H
#define C8_CHIP8_H

#include "common.h"
#include "graphics.h"

#include <stdint.h>

/**
 * @brief Maximum instructions to execute per second
 */
#define C8_CLOCK_SPEED 1000

/**
 * @brief Maximum stack size
 */
#define C8_STACK_SIZE 16

/**
 * @brief CHIP-8 execution mode. SCHIP and XO-CHIP instructions will throw an error.
 */
#define C8_MODE_CHIP8 0

/**
 * @brief SCHIP execution mode.
 */
#define C8_MODE_SCHIP 1

/**
 * @brief XO-CHIP execution mode.
 */
#define C8_MODE_XOCHIP 2

/**
 * @brief Enable debug mode.
 */
#define C8_FLAG_DEBUG 0x1

/**
 * @brief Print all instructions as they are executed.
 */
#define C8_FLAG_VERBOSE 0x2

/**
 * @brief Enable the 'b' quirk (see main page/README).
 */
#define C8_FLAG_QUIRK_BITWISE 0x4

/**
 * @brief Enable the 'd' quirk (see main page/README).
 */
#define C8_FLAG_QUIRK_DRAW 0x8

/**
 * @brief Enable the 'l' quirk (see main page/README).
 */
#define C8_FLAG_QUIRK_LOADSTORE 0x10

/**
 * @brief Enable the 's' quirk (see main page/README).
 */
#define C8_FLAG_QUIRK_SHIFT 0x20

/**
 * @brief Enable the 'j' quirk (see main page/README).
 */
#define C8_FLAG_QUIRK_JUMP 0x40

/**
  * @struct C8
  * @brief Represents current state of the CHIP-8 interpreter
  *
  * @param mem CHIP-8 memory
  * @param R flag registers
  * @param V V (general purpose) registers
  * @param sp stack pointer
  * @param dt display timer
  * @param st sound timer
  * @param stack stack
  * @param pc program counter
  * @param I I (address) register
  * @param key key press states
  * @param VK V to store next keypress
  * @param cs instructions to execute per second
  * @param waitingForKey 1 or 0
  * @param running 1 or 0
  * @param display graphics display
  * @param flags CLI flags
  * @param breakpoints debug breakpoint map
  * @param colors 24 bit hex colors, background=[0] foreground=[1]
  * @param fonts font IDs (see font.c)
  * @param draw need to draw? (1 or 0)
  * @param mode interpreter mode (C8_MODE_CHIP8, C8_MODE_SCHIP, C8_MODE_XOCHIP)
  */
typedef struct {
    uint8_t    mem[C8_MEMSIZE];
    uint8_t    R[8];
    uint8_t    V[16];
    uint8_t    sp;
    uint8_t    dt;
    uint8_t    st;
    uint16_t   stack[C8_STACK_SIZE];
    uint16_t   pc;
    uint16_t   I;
    int        key[18];
    int        VK;
    int        cs;
    int        waitingForKey;
    int        running;
    C8_Display display;
    int        flags;
    int        breakpoints[C8_MEMSIZE];
    int        colors[2];
    int        fonts[2];
    int        draw;
    int        mode;
} C8;

void        c8_deinit(C8*);
C8*         c8_init(const char*, int);
int         c8_load_palette_s(C8*, char*);
int         c8_load_palette_f(C8*, const char*);
void        c8_load_quirks(C8*, const char*);
int         c8_load_rom(C8*, const char*);
const char* c8_version(void);
void        c8_simulate(C8*);

#endif
