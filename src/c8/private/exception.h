/**
 * @file c8/private/exception.h
 * @note NOT EXPORTED
 *
 * Stuff for handling exceptions.
 */

#ifndef C8_EXCEPTION_H
#define C8_EXCEPTION_H

#include <stdio.h>

/**
 * @brief Maximum size of exception message
 */
#define C8_EXCEPTION_MESSAGE_SIZE BUFSIZ

/**
 * @brief Exception handling macro
 */
#define C8_EXCEPTION(code, ...)                                                                    \
    {                                                                                              \
        snprintf(c8_exception, C8_EXCEPTION_MESSAGE_SIZE, __VA_ARGS__);                            \
        c8_handle_exception(code);                                                                 \
    }

/**
 * @brief Exception code enum
 */
typedef enum {
    C8_INVALID_INSTRUCTION_EXCEPTION            = -3,
    C8_TOO_MANY_LABELS_EXCEPTION                = -4,
    C8_STACK_OVERFLOW_EXCEPTION                 = -5,
    C8_INVALID_ARGUMENT_EXCEPTION               = -6,
    C8_DUPLICATE_LABEL_EXCEPTION                = -7,
    C8_INVALID_SYMBOL_EXCEPTION                 = -8,
    C8_MEMORY_ALLOCATION_EXCEPTION              = -9,
    C8_UNKNOWN_EXCEPTION                        = -10,
    C8_TOO_MANY_SYMBOLS_EXCEPTION               = -11,
    C8_LOAD_FILE_FAILURE_EXCEPTION              = -12,
    C8_FILE_TOO_BIG_EXCEPTION                   = -13,
    C8_INVALID_COLOR_PALETTE_EXCEPTION          = -14,
    C8_INVALID_QUIRK_EXCEPTION                  = -15,
    C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION = -16,
    C8_INVALID_FONT_EXCEPTION                   = -17,
    C8_INVALID_CLOCK_SPEED_EXCEPTION            = -18,
    C8_STACK_UNDERFLOW_EXCEPTION                = -19
} C8_ExceptionCode;

/**
 * @brief Message to print when calling `c8_handle_exception` with a non-zero code
 */
extern char c8_exception[C8_EXCEPTION_MESSAGE_SIZE];

void        c8_handle_exception(C8_ExceptionCode);

#endif
