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
    C8_SYNTAX_ERROR_EXCEPTION      = -3,
    C8_STACK_OVERFLOW_EXCEPTION    = -4,
    C8_STACK_UNDERFLOW_EXCEPTION   = -5,
    C8_IO_EXCEPTION                = -6,
    C8_INVALID_STATE_EXCEPTION     = -7,
    C8_INVALID_PARAMETER_EXCEPTION = -8,
    C8_GRAPHICS_EXCEPTION          = -9,
    C8_AUDIO_EXCEPTION             = -10,
} C8_ExceptionCode;

/**
 * @brief Message to print when calling `c8_handle_exception` with a non-zero code
 */
extern char c8_exception[C8_EXCEPTION_MESSAGE_SIZE];

void        c8_handle_exception(C8_ExceptionCode);

#endif
