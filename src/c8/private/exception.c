/**
 * @file c8/private/exception.c
 * @note NOT EXPORTED
 *
 * Stuff for handling exceptions.
 */

#include "exception.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C8_SYNTAX_ERROR_EXCEPTION_MESSAGE      "A syntax error was encountered in the input file."
#define C8_STACK_OVERFLOW_EXCEPTION_MESSAGE    "A stack overflow occurred during execution."
#define C8_STACK_UNDERFLOW_EXCEPTION_MESSAGE   "A stack underflow occurred during execution."
#define C8_IO_EXCEPTION_MESSAGE                "A file I/O error occurred during execution."
#define C8_INVALID_STATE_EXCEPTION_MESSAGE     "The c8 instance is in an invalid state."
#define C8_INVALID_PARAMETER_EXCEPTION_MESSAGE "An invalid parameter was passed to a function."
#define C8_GRAPHICS_EXCEPTION_MESSAGE          "An error occurred while rendering graphics."
#define C8_AUDIO_EXCEPTION_MESSAGE             "An error occurred while playing audio."

/**
 * @struct C8_Exception
 * @brief  Represents an exception.
 *
 * @param code    exception code
 * @param message exception message
 */
typedef struct {
    C8_ExceptionCode code;
    const char*      message;
} C8_Exception;

const C8_Exception c8_exceptions[] = {
    { C8_SYNTAX_ERROR_EXCEPTION, C8_SYNTAX_ERROR_EXCEPTION_MESSAGE },
    { C8_STACK_OVERFLOW_EXCEPTION, C8_STACK_OVERFLOW_EXCEPTION_MESSAGE },
    { C8_STACK_UNDERFLOW_EXCEPTION, C8_STACK_UNDERFLOW_EXCEPTION_MESSAGE },
    { C8_IO_EXCEPTION, C8_IO_EXCEPTION_MESSAGE },
    { C8_INVALID_STATE_EXCEPTION, C8_INVALID_STATE_EXCEPTION_MESSAGE },
    { C8_INVALID_PARAMETER_EXCEPTION, C8_INVALID_PARAMETER_EXCEPTION_MESSAGE },
    { C8_GRAPHICS_EXCEPTION, C8_GRAPHICS_EXCEPTION_MESSAGE },
    { C8_AUDIO_EXCEPTION, C8_AUDIO_EXCEPTION_MESSAGE },
};

char c8_exception[C8_EXCEPTION_MESSAGE_SIZE];

/**
 * @brief Handles an exception by printing the corresponding error message to stderr.
 *
 * @param code The exception code.
 */
void c8_handle_exception(C8_ExceptionCode code) {
    for (size_t i = 0; i < sizeof(c8_exceptions) / sizeof(C8_Exception); i++) {
        if (c8_exceptions[i].code == code) {
            fprintf(stderr, "libc8: %s\n", c8_exceptions[i].message);
        }
    }

    fprintf(stderr, "libc8: %s\n", c8_exception);
}
