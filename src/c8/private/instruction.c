/**
 * @file c8/private/instruction.c
 * @note NOT EXPORTED
 *
 * This file contains the implementation of the instruction parsing and
 * execution for the CHIP-8 interpreter.
 */
#include "instruction.h"

#include "../chip8.h"
#include "../common.h"
#include "../decode.h"
#include "../font.h"
#include "../graphics.h"
#include "exception.h"

#include <stdlib.h>
#include <string.h>

#define C8_VERBOSE(c) (c->flags & C8_FLAG_VERBOSE)

#define C8_SCHIP_EXCLUSIVE(c)                                                                      \
    if (c->mode == C8_MODE_CHIP8) {                                                                \
        fprintf(stderr, "SCHIP instruction detected in CHIP-8 mode.\n");                           \
        return C8_INVALID_STATE_EXCEPTION;                                                         \
    }

#define C8_XOCHIP_EXCLUSIVE(c)                                                                     \
    if (c->mode != C8_MODE_XOCHIP) {                                                               \
        const char* modeStr = (c->mode == C8_MODE_CHIP8) ? "CHIP-8" : "SCHIP";                     \
        fprintf(stderr, "XOCHIP instruction detected in %s mode.\n", modeStr);                     \
        return C8_INVALID_STATE_EXCEPTION;                                                         \
    }

#define C8_QUIRK_VF_RESET(c)                                                                       \
    if (c->flags & C8_FLAG_QUIRK_VF_RESET) {                                                       \
        c->V[0xF] = 0;                                                                             \
    }

#define C8_QUIRK_MEMORY(c)                                                                         \
    if (c->flags & C8_FLAG_QUIRK_MEMORY) {                                                         \
        c8->I = (c8->I + x + 1) & 0xFFF;                                                           \
    }

#define C8_QUIRK_SHIFTING(c)                                                                       \
    if (c->flags & C8_FLAG_QUIRK_SHIFTING) {                                                       \
        y = x;                                                                                     \
    }

/* instruction groups */
C8_STATIC int           c8_base_instruction(C8*, uint16_t, uint8_t);
C8_STATIC int           c8_bitwise_instruction(C8*, uint16_t, uint8_t, uint8_t, uint8_t);
C8_STATIC int           c8_key_instruction(C8*, uint16_t, uint8_t, uint8_t);
C8_STATIC int           c8_misc_instruction(C8*, uint16_t, uint8_t, uint8_t);

C8_STATIC C8_INLINE int c8_i_scd_b(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_scu_b(C8*, uint8_t);

/* base (00kk) instructions */
C8_STATIC C8_INLINE int c8_i_cls(C8*);
C8_STATIC C8_INLINE int c8_i_ret(C8*);
C8_STATIC C8_INLINE int c8_i_scr(C8*);
C8_STATIC C8_INLINE int c8_i_scl(C8*);
C8_STATIC C8_INLINE int c8_i_exit(C8*);
C8_STATIC C8_INLINE int c8_i_low(C8*);
C8_STATIC C8_INLINE int c8_i_high(C8*);

C8_STATIC C8_INLINE int c8_i_jp_nnn(C8*, uint16_t);
C8_STATIC C8_INLINE int c8_i_call_nnn(C8*, uint16_t);
C8_STATIC C8_INLINE int c8_i_se_vx_kk(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_sne_vx_kk(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_se_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_i_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_vx_vy_i(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_vx_kk(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_add_vx_kk(C8*, uint8_t, uint8_t);

/* bitwise (8xyb) instructions */
C8_STATIC C8_INLINE int c8_i_ld_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_or_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_and_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_xor_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_add_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_sub_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_shr_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_subn_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_shl_vx_vy(C8*, uint8_t, uint8_t);

C8_STATIC C8_INLINE int c8_i_sne_vx_vy(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_i_nnn(C8*, uint16_t);
C8_STATIC C8_INLINE int c8_i_jp_v0_nnn(C8*, uint16_t);
C8_STATIC C8_INLINE int c8_i_rnd_vx_kk(C8*, uint8_t, uint8_t);
C8_STATIC C8_INLINE int c8_i_drw_vx_vy_b(C8*, uint8_t, uint8_t, uint8_t);

/* key (Ex00) instructions */
C8_STATIC C8_INLINE int c8_i_skp_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_sknp_vx(C8*, uint8_t);

/* misc (Fxkk) instructions */
C8_STATIC C8_INLINE int c8_i_ld_i_word(C8*);
C8_STATIC C8_INLINE int c8_i_pln_x(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_snd(C8*);
C8_STATIC C8_INLINE int c8_i_ld_vx_dt(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_vx_k(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_dt_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_st_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_add_i_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_f_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_hf_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_b_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_pit_x(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_ip_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_vx_ip(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_r_vx(C8*, uint8_t);
C8_STATIC C8_INLINE int c8_i_ld_vx_r(C8*, uint8_t);

/**
 * @brief Execute the instruction at `c8->pc`
 *
 * This function parses and executes the instruction at the current program
 * counter.
 *
 * If verbose flag is set, this will print the instruction to `stdout` as well.
 *
 * @param c8 the `C8` to execute the instruction from
 * @return amount to increase the program counter, or an exception code if an
 * error occurs.
 */
int c8_parse_instruction(C8* c8) {
    uint16_t in = (((uint16_t) c8->mem[c8->pc]) << 8) | c8->mem[c8->pc + 1];
    C8_EXPAND(in);

    if (C8_VERBOSE(c8)) {
        printf("%04x: %s\n", c8->pc, c8_decode_instruction(in, NULL));
    }

    switch (a) {
    case 0x0:
        return c8_base_instruction(c8, in, kk);
    case 0x1:
        return c8_i_jp_nnn(c8, nnn);
    case 0x2:
        return c8_i_call_nnn(c8, nnn);
    case 0x3:
        return c8_i_se_vx_kk(c8, x, kk);
    case 0x4:
        return c8_i_sne_vx_kk(c8, x, kk);
    case 0x5:
        return c8_i_se_vx_vy(c8, x, y);
    case 0x6:
        return c8_i_ld_vx_kk(c8, x, kk);
    case 0x7:
        return c8_i_add_vx_kk(c8, x, kk);
    case 0x8:
        return c8_bitwise_instruction(c8, in, x, y, b);
    case 0x9:
        return c8_i_sne_vx_vy(c8, x, y);
    case 0xA:
        return c8_i_ld_i_nnn(c8, nnn);
    case 0xB:
        return c8_i_jp_v0_nnn(c8, nnn);
    case 0xC:
        return c8_i_rnd_vx_kk(c8, x, kk);
    case 0xD:
        return c8_i_drw_vx_vy_b(c8, x, y, b);
    case 0xE:
        return c8_key_instruction(c8, in, x, kk);
    case 0xF:
        return c8_misc_instruction(c8, in, x, kk);
    default: // unreachable
        C8_EXCEPTION(C8_SYNTAX_ERROR_EXCEPTION, "Unreachable Invalid instruction: %04x", in);
        return C8_SYNTAX_ERROR_EXCEPTION;
    }
}

C8_STATIC C8_INLINE int c8_base_instruction(C8* c8, uint16_t in, uint8_t kk) {
    if (in & 0x0F00) {
        C8_EXCEPTION(C8_SYNTAX_ERROR_EXCEPTION, "Invalid instruction: %04x", in);
        return C8_SYNTAX_ERROR_EXCEPTION;
    }

    if (C8_Y(in) == 0xC) {
        return c8_i_scd_b(c8, kk);
    }
    if (C8_Y(in) == 0xD) {
        return c8_i_scu_b(c8, kk);
    }

    switch (kk) {
    case 0xE0:
        return c8_i_cls(c8);
    case 0xEE:
        return c8_i_ret(c8);
    case 0xFB:
        return c8_i_scr(c8);
    case 0xFC:
        return c8_i_scl(c8);
    case 0xFD:
        return c8_i_exit(c8);
    case 0xFE:
        return c8_i_low(c8);
    case 0xFF:
        return c8_i_high(c8);
    default:
        C8_EXCEPTION(C8_SYNTAX_ERROR_EXCEPTION, "Invalid instruction: %04x", in);
        return C8_SYNTAX_ERROR_EXCEPTION;
    }
}

/**
 * @brief Handles a bitwise instruction by calling the appropriate function.
 *
 * @param c8 The current CHIP-8 state.
 * @param in The instruction word.
 * @param x The x register.
 * @param y The y register.
 * @param b The bitwise operation code.
 * @return The result of the bitwise operation.
 */
C8_STATIC C8_INLINE int
c8_bitwise_instruction(C8* c8, uint16_t in, uint8_t x, uint8_t y, uint8_t b) {
    switch (b) {
    case 0x0:
        return c8_i_ld_vx_vy(c8, x, y);
    case 0x1:
        return c8_i_or_vx_vy(c8, x, y);
    case 0x2:
        return c8_i_and_vx_vy(c8, x, y);
    case 0x3:
        return c8_i_xor_vx_vy(c8, x, y);
    case 0x4:
        return c8_i_add_vx_vy(c8, x, y);
    case 0x5:
        return c8_i_sub_vx_vy(c8, x, y);
    case 0x6:
        return c8_i_shr_vx_vy(c8, x, y);
    case 0x7:
        return c8_i_subn_vx_vy(c8, x, y);
    case 0xE:
        return c8_i_shl_vx_vy(c8, x, y);
    default:
        C8_EXCEPTION(C8_SYNTAX_ERROR_EXCEPTION, "Invalid instruction: %04x", in);
        return C8_SYNTAX_ERROR_EXCEPTION;
    }
}

/**
 * Handles a key instruction by calling the appropriate function.
 *
 * @param c8 The current CHIP-8 state.
 * @param in The instruction word.
 * @param x The x register.
 * @param kk The key code.
 * @return The result of the key instruction.
 */
C8_STATIC C8_INLINE int c8_key_instruction(C8* c8, uint16_t in, uint8_t x, uint8_t kk) {
    switch (kk) {
    case 0x9E:
        return c8_i_skp_vx(c8, x);
    case 0xA1:
        return c8_i_sknp_vx(c8, x);
    default:
        C8_EXCEPTION(C8_SYNTAX_ERROR_EXCEPTION, "Invalid instruction: %04x", in);
        return C8_SYNTAX_ERROR_EXCEPTION;
    }
}

/**
 * Handles a miscellaneous instruction by calling the appropriate function.
 *
 * @param c8 The current CHIP-8 state.
 * @param in The instruction word.
 * @param x The x register.
 * @param kk The key code.
 * @return The result of the miscellaneous instruction.
 */
C8_STATIC C8_INLINE int c8_misc_instruction(C8* c8, uint16_t in, uint8_t x, uint8_t kk) {
    if (in == 0xF000) {
        return c8_i_ld_i_word(c8);
    } else if (in == 0xF002) {
        return c8_i_snd(c8);
    }

    switch (kk) {
    case 0x01:
        return c8_i_pln_x(c8, x);
    case 0x07:
        return c8_i_ld_vx_dt(c8, x);
    case 0x0A:
        return c8_i_ld_vx_k(c8, x);
    case 0x15:
        return c8_i_ld_dt_vx(c8, x);
    case 0x18:
        return c8_i_ld_st_vx(c8, x);
    case 0x1E:
        return c8_i_add_i_vx(c8, x);
    case 0x29:
        return c8_i_ld_f_vx(c8, x);
    case 0x30:
        return c8_i_ld_hf_vx(c8, x);
    case 0x33:
        return c8_i_ld_b_vx(c8, x);
    case 0x3A:
        return c8_i_pit_x(c8, x);
    case 0x55:
        return c8_i_ld_ip_vx(c8, x);
    case 0x65:
        return c8_i_ld_vx_ip(c8, x);
    case 0x75:
        return c8_i_ld_r_vx(c8, x);
    case 0x85:
        return c8_i_ld_vx_r(c8, x);
    default:
        C8_EXCEPTION(C8_SYNTAX_ERROR_EXCEPTION, "Invalid instruction: %04x", in);
        return C8_SYNTAX_ERROR_EXCEPTION;
    }
}

/**
 * @brief `SCD b` instruction (`00Cb`)
 *
 * This instruction scrolls the display down by `b` pixels.
 *
 * @note This is a SCHIP instruction. `c8` must be in SCHIP or XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param b the number of pixels to scroll down
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_scd_b(C8* c8, uint8_t b) {
    C8_SCHIP_EXCLUSIVE(c8);

    int width
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_WIDTH : C8_HIGH_DISPLAY_WIDTH;
    int height
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_HEIGHT : C8_HIGH_DISPLAY_HEIGHT;

    memcpy(c8->display.p + (width * b), c8->display.p, width * height - (width * b));
    memset(c8->display.p, 0, width * b);
    return 2;
}

/**
 * @brief `SCU b` instruction (`00Db`)
 *
 * This instruction scrolls the display up by `b` pixels.
 *
 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param b the number of pixels to scroll up
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_scu_b(C8* c8, uint8_t b) {
    C8_SCHIP_EXCLUSIVE(c8);

    int width
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_WIDTH : C8_HIGH_DISPLAY_WIDTH;
    int height
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_HEIGHT : C8_HIGH_DISPLAY_HEIGHT;

    memcpy(c8->display.p, c8->display.p + (width * b), width * height - (width * b));
    memset(c8->display.p + (width * (height - b)), 0, width * b);
    return 2;
}

/**
 * @brief `CLS` instruction (`00E0`)
 *
 * This instruction clears the display and sets the draw flag.
 *
 * @param c8 the `C8` to execute the instruction from
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_cls(C8* c8) {
    memset(&c8->display.p, 0, C8_HIGH_DISPLAY_WIDTH * C8_HIGH_DISPLAY_HEIGHT);
    return 2;
}

/**
 * @brief `RET` instruction (`00EE`)
 *
 * This instruction pops the top of the stack and sets the program counter to it.
 * Usually, the PC should be at a `CALL` instruction after this, so the program
 * counter must be increased by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 2, the number of bytes to increase the program counter by,
 * or STACK_UNDERFLOW_EXCEPTION if the stack is empty.
 */
C8_STATIC C8_INLINE int c8_i_ret(C8* c8) {
    if (c8->sp == 0) {
        C8_EXCEPTION(C8_STACK_UNDERFLOW_EXCEPTION, "Stack underflow at %03x", c8->pc);
        return C8_STACK_UNDERFLOW_EXCEPTION;
    }
    c8->sp--;
    c8->pc = c8->stack[c8->sp];
    return 2;
}

/**
 * @brief `SCR` instruction (`00FB`)
 *
 * This instruction scrolls the display right by 4 pixels.
 *
 * @note This is a SCHIP instruction. `c8` must be in SCHIP or XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_scr(C8* c8) {
    C8_SCHIP_EXCLUSIVE(c8);

    int width
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_WIDTH : C8_HIGH_DISPLAY_WIDTH;
    int height
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_HEIGHT : C8_HIGH_DISPLAY_HEIGHT;

    for (int y = 0; y < height; y++) {
        for (int x = width - 4 - 1; x >= 0; x--) {
            int orig           = y * width + x;
            int new            = orig + 4;
            c8->display.p[new] = c8->display.p[orig];
        }
        memset(&c8->display.p[y * width], 0, 4);
    }
    return 2;
}

/**
 * @brief `SCL` instruction (`00FC`)
 *
 * This instruction scrolls the display left by 4 pixels.
 *
 * @note This is a SCHIP instruction. `c8` must be in SCHIP or XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_scl(C8* c8) {
    C8_SCHIP_EXCLUSIVE(c8);

    int width
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_WIDTH : C8_HIGH_DISPLAY_WIDTH;
    int height
        = (c8->display.mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_HEIGHT : C8_HIGH_DISPLAY_HEIGHT;

    for (int y = 0; y < height; y++) {
        for (int x = 4; x < width; x++) {
            int orig           = y * width + x;
            int new            = orig - 4;
            c8->display.p[new] = c8->display.p[orig];
        }
        memset(&c8->display.p[y * width + width - 4], 0, 4);
    }
    return 2;
}

/**
 * @brief `EXIT` instruction (`00FD`)
 *
 * This instruction stops the interpreter from running.
 *
 * @note This is a SCHIP instruction. `c8` must be in SCHIP or XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 0, or C8_INVALID_INSTRUCTION_EXCEPTION if `c8` is in SCHIP mode.
 */
C8_STATIC C8_INLINE int c8_i_exit(C8* c8) {
    C8_SCHIP_EXCLUSIVE(c8);
    c8->running = 0;
    return 0;
}

/**
 * @brief `LOW` instruction (`00FE`)
 *
 * This instruction sets the display mode to low resolution (64x32).
 *
 * @note This is a SCHIP instruction. `c8` must be in SCHIP or XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 2, the number of bytes to increase the program counter by, or
 * C8_INVALID_INSTRUCTION_EXCEPTION if `c8` is in CHIP-8 mode.
 */
C8_STATIC C8_INLINE int c8_i_low(C8* c8) {
    C8_SCHIP_EXCLUSIVE(c8);
    c8->display.mode = C8_DISPLAYMODE_LOW;
    return 2;
}

/**
 * @brief `HIGH` instruction (`00FF`)
 *
 * This instruction sets the display mode to high resolution (128x64).
 *
 * @note This is a SCHIP instruction. `c8` must be in SCHIP or XO-CHIP mode to
 * execute this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 2, the number of bytes to increase the program counter by, or
 * C8_INVALID_INSTRUCTION_EXCEPTION if `c8` is in CHIP-8 mode.
 */
C8_STATIC C8_INLINE int c8_i_high(C8* c8) {
    C8_SCHIP_EXCLUSIVE(c8);
    c8->display.mode = C8_DISPLAYMODE_HIGH;
    return 2;
}

/**
 * @brief `JP nnn` instruction (`1nnn`)
 *
 * This instruction sets the program counter to `nnn`.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param nnn the address to jump to
 *
 * @return 0, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_jp_nnn(C8* c8, uint16_t nnn) {
    c8->pc = nnn;
    return 0;
}

/**
 * @brief `CALL nnn` instruction (`2nnn`)
 *
 * This instruction pushes the current program counter onto the stack and
 * sets the program counter to `nnn`.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param nnn the address to call
 *
 * @return 0, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_call_nnn(C8* c8, uint16_t nnn) {
    if (c8->sp >= 15) {
        C8_EXCEPTION(C8_STACK_OVERFLOW_EXCEPTION, "Stack overflow at %03x", c8->pc);
        return C8_STACK_OVERFLOW_EXCEPTION;
    }
    c8->stack[c8->sp] = c8->pc;
    c8->sp++;
    c8->pc = nnn;
    return 0;
}

/**
 * @brief `SE Vx, kk` instruction (`3xkk`)
 *
 * This instruction checks if the value in register Vx is equal to kk.
 * If they are equal, it increases the program counter by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param kk the byte value to compare against
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_se_vx_kk(C8* c8, uint8_t x, uint8_t kk) {
    if (c8->V[x] == kk) {
        c8->pc += 2;
    }
    return 2;
}

/**
 * @brief `SNE Vx, kk` instruction (4xkk)
 *
 * This instruction checks if the value in register Vx is not equal to kk.
 * If they are not equal, it increases the program counter by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param kk the byte value to compare against
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_sne_vx_kk(C8* c8, uint8_t x, uint8_t kk) {
    if (c8->V[x] != kk) {
        c8->pc += 2;
    }
    return 2;
}

/**
 * @brief `SE Vx, Vy` instruction (`5xy0`)
 *
 * This instruction checks if the value in register Vx is equal to the value in
 * register Vy. If they are not equal, it increases the program counter by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_se_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    if (c8->V[x] == c8->V[y]) {
        c8->pc += 2;
    }
    return 2;
}

/**
 * @brief `LD [I], Vx, Vy` instruction (`5xy2`)
 *
 * This instruction saves the values in registers Vx-Vy to memory starting at the
 * address I.

 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to execute
 * this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_i_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    C8_XOCHIP_EXCLUSIVE(c8);

    for (int i = x; i <= y; i++) {
        c8->mem[c8->I + x] = c8->V[i];
    }
    return 2;
}

/**
 * @brief `LD Vx, Vy, [I]` instruction (`5xy3`)
 *
 * This instruction loads values from memory to the registers Vx-Vy starting at the
 * address I.

 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to execute
 * this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_vy_i(C8* c8, uint8_t x, uint8_t y) {
    C8_XOCHIP_EXCLUSIVE(c8);

    for (int i = x; i <= y; i++) {
        c8->V[i] = c8->mem[c8->I + x];
    }
    return 2;
}

/**
 * @brief `LD Vx, kk` instruction (`6xkk`)
 *
 * This instruction loads the value `kk` into register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param kk the byte value to load into Vx
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_kk(C8* c8, uint8_t x, uint8_t kk) {
    c8->V[x] = kk;
    return 2;
}

/**
 * @brief `ADD Vx, kk` instruction (`7xkk`)
 *
 * This instruction adds the value `kk` to the value in register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param kk the byte value to add to Vx
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_add_vx_kk(C8* c8, uint8_t x, uint8_t kk) {
    c8->V[x] += kk;
    return 2;
}

/**
 * @brief `LD Vx, Vy` instruction (`8xy0`)
 *
 * This instruction loads the value in register Vy into register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    c8->V[x] = c8->V[y];
    return 2;
}

/**
 * @brief `OR Vx, Vy` instruction (`8xy1`)
 *
 * This instruction performs a bitwise OR operation between the values in registers Vx and Vy,
 * and stores the result in register Vx.
 *
 * @note This instruction is affected by the QUIRK_BITWISE flag. If the flag is
 * set in `c8.quirks`, VF is set to 0.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_or_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    c8->V[x] |= c8->V[y];
    C8_QUIRK_VF_RESET(c8);
    return 2;
}

/**
 * @brief `AND Vx, Vy` instruction (`8xy2`)
 *
 * This instruction performs a bitwise AND operation between the values in registers Vx and Vy,
 * and stores the result in register Vx.
 *
 * @note This instruction is affected by the QUIRK_BITWISE flag. If the flag is
 * set in `c8.quirks`, VF is set to 0.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_and_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    c8->V[x] &= c8->V[y];
    C8_QUIRK_VF_RESET(c8);
    return 2;
}

/**
 * @brief `XOR Vx, Vy` instruction (`8xy3`)
 *
 * This instruction performs a bitwise XOR operation between the values in registers Vx and Vy,
 * and stores the result in register Vx.
 *
 * @note This instruction is affected by the QUIRK_BITWISE flag. If the flag is
 * set in `c8.quirks`, VF is set to 0.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_xor_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    c8->V[x] = c8->V[x] ^ c8->V[y];
    C8_QUIRK_VF_RESET(c8);
    return 2;
}

/**
 * @brief `ADD Vx, Vy` instruction (`8xy4`)
 *
 * This instruction adds the value in register Vy to the value in register Vx.
 * If the result exceeds 255, it sets VF to 1.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_add_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    uint16_t sum = c8->V[x] + c8->V[y];
    c8->V[x]     = sum;
    c8->V[0xF]   = (sum > 0xFF) ? 1 : 0;
    return 2;
}

/**
 * @brief `SUB Vx, Vy` instruction (`8xy5`)
 *
 * This instruction subtracts the value in register Vy from the value in register Vx.
 * If the result is not negative, it sets VF to 1.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_sub_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    uint8_t result = c8->V[x] - c8->V[y];
    uint8_t vf     = result <= c8->V[x];
    c8->V[x]       = result;
    c8->V[0xF]     = vf;
    return 2;
}

void print_binary(unsigned int n) {
    for (int i = 31; i >= 0; i--) {
        int k = n >> i;
        if (k & 1)
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

/**
 * @brief `SHR Vx, Vy` instruction (`8xy6`)
 *
 * This instruction shifts the value in register Vy right by 1 bit and stores
 * the result in register Vx. The least significant bit of the result is stored
 * in VF.
 *
 * @note This instruction is affected by the QUIRK_SHIFT flag. If the flag is
 * set in `c8.quirks`, the value of y is set to x, effectively making it a right
 * shift of Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_shr_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    C8_QUIRK_SHIFTING(c8);
    uint8_t vy = c8->V[y];
    c8->V[x]   = c8->V[y] >> 1;
    c8->V[0xF] = vy & 0x1;
    return 2;
}

/**
 * @brief `SUBN Vx, Vy` instruction (`8xy7`)
 *
 * This instruction subtracts the value in register Vx from the value in register Vy.
 * If the result is not negative, it sets VF to 1.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_subn_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    uint8_t vf = c8->V[x] < c8->V[y];
    c8->V[x]   = c8->V[y] - c8->V[x];
    c8->V[0xF] = vf;
    return 2;
}

/**
 * @brief `SHL Vx, Vy` instruction (`8xyE`)
 *
 * This instruction shifts the value in register Vy left by 1 bit and stores
 * the result in register Vx. The most significant bit of the result is stored
 * in VF.
 *
 * @note This instruction is affected by the QUIRK_SHIFT flag. If the flag is
 * set in `c8.quirks`, the value of y is set to x, effectively making it a left
 * shift of Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_shl_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    C8_QUIRK_SHIFTING(c8);
    uint8_t vy = c8->V[y];
    c8->V[x]   = c8->V[y] << 1;
    c8->V[0xF] = vy >> 7 & 1;
    return 2;
}

/**
 * @brief `SNE Vx, Vy` instruction (`9xy0`)
 *
 * This instruction checks if the value in register Vx is not equal to the value in register Vy.
 * If they are not equal, it increases the program counter by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_sne_vx_vy(C8* c8, uint8_t x, uint8_t y) {
    if (c8->V[x] != c8->V[y]) {
        c8->pc += 2;
    }
    return 2;
}

/**
 * @brief `LD I, nnn` instruction (`Annn`)
 *
 * This instruction sets the index register I to the value nnn.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param nnn the address to set I to
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_i_nnn(C8* c8, uint16_t nnn) {
    c8->I = nnn;
    return 2;
}

/**
 * @brief `JP V0, nnn` instruction (`Bnnn`)
 *
 * This instruction sets the program counter to `nnn` plus the value in register V0.
 *
 * @note If the QUIRK_JUMP flag is set in `c8->flags`, it uses the value in
 * register V[8th bit of nnn] instead of V0.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param nnn the address to jump to
 *
 * @return 0, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_jp_v0_nnn(C8* c8, uint16_t nnn) {
    if (c8->flags & C8_FLAG_QUIRK_JUMPING) {
        c8->pc = nnn + c8->V[(nnn >> 8) & 0xF];
    } else {
        c8->pc = nnn + c8->V[0];
    }
    return 0;
}

/**
 * @brief `RND Vx, kk` instruction (`Cxkk`)
 *
 * This instruction generates a random number and performs a bitwise AND operation
 * with `kk`, storing the result in register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param kk the byte value to AND with the random number
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_rnd_vx_kk(C8* c8, uint8_t x, uint8_t kk) {
    c8->V[x] = rand() & kk;
    return 2;
}

/**
 * @brief `DRW Vx, Vy, b` instruction (`Dxyb`)
 *
 * This instruction draws a sprite at the coordinates specified by the values in
 * registers Vx and Vy. The sprite is `sprite_width` bytes long, starting from the address
 * in the index register I. The sprite is XOR'd onto the display, and if any
 * pixels are turned off that were previously on, the VF register is set to 1.
 * Then the draw flag is set to 1.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @param y the index of the register Vy (0-15)
 * @param b the number of bytes in the sprite (1-16). Also serves as the height of the sprite.
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_drw_vx_vy_b(C8* c8, uint8_t x, uint8_t y, uint8_t b) {
    uint8_t vf             = 0;
    int     display_width  = C8_LOW_DISPLAY_WIDTH;
    int     display_height = C8_LOW_DISPLAY_HEIGHT;
    int     sprite_width   = 8;

    if (c8->display.mode == C8_DISPLAYMODE_HIGH) {
        if (b == 0) {
            b            = 16;
            sprite_width = 16;
        }
        display_width  = C8_HIGH_DISPLAY_WIDTH;
        display_height = C8_HIGH_DISPLAY_HEIGHT;
    }

    for (int i = 0; i < b; i++) {
        for (int j = 0; j < sprite_width; j++) {
            int display_x = (c8->V[x] + j);
            int display_y = (c8->V[y] + i);

            if (c8->flags & C8_FLAG_QUIRK_CLIPPING) {
                if ((display_x >= display_width || display_y >= display_height)
                    && (c8->V[x] < display_width && c8->V[y] < display_height)) {
                    continue;
                }
            }

            display_x %= display_width;
            display_y %= display_height;

            int before = *c8_get_pixel(&c8->display, display_x, display_y);
            int pix    = c8->mem[c8->I + i];

            if (pix & (0x80 >> j)) {
                if (before) {
                    vf = 1;
                }
                c8_get_pixel(&c8->display, display_x, display_y)[0] ^= 1;
            }
        }
    }

    c8->V[0xF] = vf;

    if (c8->flags & C8_FLAG_QUIRK_VBLANK) {
        c8->waitingForDraw = 1;
    }
    return 2;
}

/**
 * @brief `SKP Vx` instruction (`Ex9E`)
 *
 * This instruction checks if the key corresponding to the value in register Vx
 * is pressed. If it is pressed, it increases the program counter by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_skp_vx(C8* c8, uint8_t x) {
    if (c8->key[c8->V[x] & 0xF]) {
        c8->pc += 2;
    }
    return 2;
}

/**
 * @brief `SKNP Vx` instruction (`ExA1`)
 *
 * This instruction checks if the key corresponding to the value in register Vx
 * is not pressed. If it is not pressed, it increases the program counter by 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_sknp_vx(C8* c8, uint8_t x) {
    if (!c8->key[c8->V[x] & 0xF]) {
        c8->pc += 2;
    }
    return 2;
}

/**
 * @brief `LD I, NNNN` instruction (`F000` `NNNN`)
 *
 * This instruction loads the value of the address register I with a 16-bit
 * value defined in the next two bytes.
 *
 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to execute
 * this instruction.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @return 4, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_i_word(C8* c8) {
    C8_XOCHIP_EXCLUSIVE(c8);
    c8->I = (c8->mem[c8->pc + 2] << 8) | c8->mem[c8->pc + 3];
    return 4;
}

/**
 * @brief `PLN x` instruction (`Fx01`)

 * This instruction selects zero or more drawing planes by bitmask (0 <= n <= 3).
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the bitmask to apply (0-3)
 *
 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to execute
 * this instruction.
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_pln_x(C8* c8, uint8_t x) {
    // TODO implement
    return 2;
}

/**
 * @brief `SND` instruction (`F002`)
 *
 * This instruction stores 16 bits starting at `[I]` into the audio pattern
 * buffer.
 *
 * @param c8 the `C8` to execute the instruction from
 *
 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to execute
 * this instruction.
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_snd(C8* c8) {
    // TODO implement
    return 2;
}

/**
 * @brief `LD Vx, DT` instruction (`Fx07`)
 *
 * This instruction loads the value of the delay timer into register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_dt(C8* c8, uint8_t x) {
    c8->V[x] = c8->dt;
    return 2;
}

/**
 * @brief `LD Vx, K` instruction (`Fx0A`)
 *
 * This instruction waits for a key press and stores the index of the pressed
 * key in register Vx. If no key is pressed, it sets the `waitingForKey` flag
 * to 1.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 0
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_k(C8* c8, uint8_t x) {
    // Wait for a key release
    c8->VK            = x;
    c8->waitingForKey = 1;
    return 2;
}

/**
 * @brief `LD DT, Vx` instruction (`Fx15`)
 *
 * This instruction sets the delay timer to the value in register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_dt_vx(C8* c8, uint8_t x) {
    c8->dt = c8->V[x];
    return 2;
}

/**
 * @brief `LD ST, Vx` instruction (`Fx18`)
 *
 * This instruction sets the sound timer to the value in register Vx.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_st_vx(C8* c8, uint8_t x) {
    if (c8->st == 0) {
        c8_sound_stop();
    }

    c8->st = c8->V[x];
    return 2;
}

/**
 * @brief `ADD I, Vx` instruction (`Fx1E`)
 *
 * This instruction adds the value in register Vx to the index register I.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_add_i_vx(C8* c8, uint8_t x) {
    c8->I += c8->V[x];
    return 2;
}

/**
 * @brief `LD F, Vx` instruction (`Fx29`)
 *
 * This instruction sets the index register I to the address of the
 * font character corresponding to the value in register Vx.
 *
 * The font characters are stored in the range `C8_FONT_START` to
 * `C8_FONT_START` + 80.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_f_vx(C8* c8, uint8_t x) {
    c8->I = C8_FONT_START + ((c8->V[x] & 0xF) * 5);
    return 2;
}

/**
 * @brief `LD HF, Vx` instruction (`Fx30`)
 *
 * This instruction sets the index register I to the address of the
 * font character corresponding to the value in register Vx.
 *
 * The font characters are stored in the range `C8_HIGH_FONT_START` to
 * `C8_HIGH_FONT_START` + 160.
 *
 * @note This instruction is only available in SCHIP and XO-CHIP modes.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 * @return 2, the number of bytes to increase the program counter by, or
 * `C8_INVALID_INSTRUCTION_EXCEPTION` if `c8` is in CHIP-8 mode.
 */
C8_STATIC C8_INLINE int c8_i_ld_hf_vx(C8* c8, uint8_t x) {
    C8_SCHIP_EXCLUSIVE(c8);

    c8->I = C8_HIGH_FONT_START + (c8->V[x] * 10);
    return 2;
}

/**
 * @brief `LD I, B` instruction (`Fx33`)
 *
 * This instruction stores the binary-coded decimal representation of the value
 * in register Vx at the address in index register I. The hundreds digit is stored
 * at I, the tens digit at I + 1, and the ones digit at I + 2.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_b_vx(C8* c8, uint8_t x) {
    c8->mem[c8->I]     = (c8->V[x] / 100) % 10; // hundreds
    c8->mem[c8->I + 1] = (c8->V[x] / 10) % 10; // tens
    c8->mem[c8->I + 2] = c8->V[x] % 10; // ones
    return 2;
}

/**
 * @brief `PIT` instruction (`Fx3A`)
 *
 * This instruction sets the audio pattern playback rate to `4000*2^((Vx-64)/48)Hz`.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)

 * @note This is a XO-CHIP instruction. `c8` must be in XO-CHIP mode to execute
 * this instruction.
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_pit_x(C8* c8, uint8_t x) {
    // TODO implement
    return 2;
}

/**
 * @brief `LD [I], Vx` instruction (`Fx55`)
 *
 * This instruction stores the values of registers V0 to Vx at the address in
 * index register I. The values are stored in memory starting from address I.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the index of the register Vx (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_ip_vx(C8* c8, uint8_t x) {
    for (int i = 0; i < x + 1; i++) {
        c8->mem[c8->I + i] = c8->V[i];
    }
    C8_QUIRK_MEMORY(c8);
    return 2;
}

/**
 * @brief `LD Vx, [I]` instruction (`Fx65`)
 *
 * This instruction loads the values from memory starting at address I into
 * registers V0 to Vx. The values are loaded in order from I to I + x.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the number of registers to load (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by.
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_ip(C8* c8, uint8_t x) {
    for (int i = 0; i < x + 1; i++) {
        c8->V[i] = c8->mem[c8->I + i];
    }
    C8_QUIRK_MEMORY(c8);
    return 2;
}

/**
 * @brief `LD R, Vx` instruction (`Fx75`)
 *
 * This instruction copies the values from registers V0 to Vx into the
 * registers R0 to Rx. The values are copied in order from V0 to Vx
 * to R0 to Rx.
 *
 * @note This instruction is only available in SCHIP and XO-CHIP modes.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the number of registers to copy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by,
 * or C8_INVALID_INSTRUCTION_EXCEPTION if `c8` is in CHIP-8 mode.
 */
C8_STATIC C8_INLINE int c8_i_ld_r_vx(C8* c8, uint8_t x) {
    C8_SCHIP_EXCLUSIVE(c8);
    for (int i = 0; i < x; i++) {
        c8->R[i] = c8->V[i];
    }
    return 2;
}

/**
 * @brief `LD Vx, R` instruction (`Fx85`)
 *
 * This instruction copies the values from registers R0 to Rx into the
 * registers V0 to Vx. The values are copied in order from R0 to Rx
 * to V0 to Vx.
 *
 * @note This instruction is only available in SCHIP and XO-CHIP modes.
 *
 * @param c8 the `C8` to execute the instruction from
 * @param x the number of registers to copy (0-15)
 *
 * @return 2, the number of bytes to increase the program counter by,
 * or C8_INVALID_INSTRUCTION_EXCEPTION if `c8` is in CHIP-8 mode.
 */
C8_STATIC C8_INLINE int c8_i_ld_vx_r(C8* c8, uint8_t x) {
    C8_SCHIP_EXCLUSIVE(c8);

    for (int i = 0; i < x; i++) {
        c8->V[i] = c8->R[i];
    }
    return 2;
}
