/**
 * @file c8/common.h
 *
 * Base CHIP-8 definitions/macros
 */

#ifndef C8_COMMON_H
#define C8_COMMON_H

#ifdef TEST
#define C8_STATIC
#define C8_INLINE
#else
#define C8_STATIC static
#define C8_INLINE inline
#endif

#ifndef C8_VERSION
/**
 * @brief Version of libc8.
 */
#define C8_VERSION "unknown"
#endif

/**
 * @brief Get "x" value of the given opcode.
 */
#define C8_X(i) ((i & 0x0F00) >> 8)

/**
 * @brief Get "y" value of the given opcode.
 */
#define C8_Y(i) ((i & 0x00F0) >> 4)

/**
 * @brief Get "nnn" value of the given opcode.
 */
#define C8_NNN(i) (i & 0x0FFF)

/**
 * @brief Get "a" value of the given opcode.
 */
#define C8_A(i) ((i & 0xF000) >> 12)

/**
 * @brief Get "b" value of the given opcode.
 */
#define C8_B(i) (i & 0x000F)

/**
 * @brief Get "kk" value of the given opcode.
 */
#define C8_KK(i) (i & 0x00FF)

/**
 * @brief Define variables for conventional sections of opcode (x, y, nnn, a, b, and kk).
 */
#define C8_EXPAND(i)                                                                               \
    uint8_t  x   = C8_X(i);                                                                        \
    uint8_t  y   = C8_Y(i);                                                                        \
    uint16_t nnn = C8_NNN(i);                                                                      \
    uint8_t  a   = C8_A(i);                                                                        \
    uint8_t  b   = C8_B(i);                                                                        \
    uint8_t  kk  = C8_KK(i);

/**
 * @brief Program start address.
 */
#define C8_PROG_START 0x200

/**
 * @brief Total memory size.
 */
#define C8_MEMSIZE 0x1000

#endif
