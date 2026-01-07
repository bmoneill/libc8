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
  */
typedef struct {
    uint8_t    mem[C8_MEMSIZE]; //!< CHIP-8 memory
    uint8_t    R[8]; //!< Flag registers
    uint8_t    V[16]; //!< General purpose registers
    uint8_t    sp; //!< Stack pointer
    uint8_t    dt; //!< Delay timer
    uint8_t    st; //!< Sound timer
    uint16_t   stack[C8_STACK_SIZE]; //!< Stack
    uint16_t   pc; //!< Program counter
    uint16_t   I; //!< Address register
    int        key[18]; //!< Key press states
    int        VK; //!< Register to store next keypress
    int        cs; //!< Instructions to execute per second
    int        waitingForKey; //!< Waiting for keypress?
    int        running; //!< Interpreter running state
    C8_Display display; //!< Graphics display
    int        flags; //!< CLI flags
    int        breakpoints[C8_MEMSIZE]; //!< Debug breakpoint map
    int        colors[2]; //!< 24 bit hex colors, background=[0] foreground=[1]
    int        fonts[2]; //!< Font IDs (see font.c)
    int        draw; //!< Need to draw? (1 or 0)
    int        mode; //!< Interpreter mode (C8_MODE_CHIP8, C8_MODE_SCHIP, C8_MODE_XOCHIP)
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
