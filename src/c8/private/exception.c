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

#define C8_INVALID_INSTRUCTION_EXCEPTION_MESSAGE            "An invalid instruction exists in the input file."
#define C8_TOO_MANY_LABELS_EXCEPTION_MESSAGE                "Too many labels are defined in the input file."
#define C8_STACK_OVERFLOW_EXCEPTION_MESSAGE                 "A stack overflow occurred during execution."
#define C8_INVALID_ARGUMENT_EXCEPTION_MESSAGE               "An invalid instruction argument was given."
#define C8_DUPLICATE_LABEL_EXCEPTION_MESSAGE                "A label was defined multiple times."
#define C8_INVALID_SYMBOL_EXCEPTION_MESSAGE                 "An invalid symbol exists in the input file."
#define C8_MEMORY_ALLOCATION_EXCEPTION_MESSAGE              "Failed to allocate memory."
#define C8_UNKNOWN_EXCEPTION_MESSAGE                        "An unknown error has occurred."
#define C8_TOO_MANY_SYMBOLS_EXCEPTION_MESSAGE               "Too many symbols exist in the input file."
#define C8_LOAD_FILE_FAILURE_EXCEPTION_MESSAGE              "Failed to load file."
#define C8_FILE_TOO_BIG_EXCEPTION_MESSAGE                   "The given file is too big."
#define C8_INVALID_COLOR_PALETTE_EXCEPTION_MESSAGE          "Invalid color palette."
#define C8_INVALID_QUIRK_EXCEPTION_MESSAGE                  "Invalid quirk."
#define C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION_MESSAGE "Failed to initialize graphics."
#define C8_INVALID_FONT_EXCEPTION_MESSAGE                   "Invalid font."
#define C8_INVALID_CLOCK_SPEED_EXCEPTION_MESSAGE            "Clock speed cannot be less than 1."
#define C8_STACK_UNDERFLOW_EXCEPTION_MESSAGE                "Stack underflow occurred during execution."

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
    { C8_INVALID_INSTRUCTION_EXCEPTION, C8_INVALID_INSTRUCTION_EXCEPTION_MESSAGE },
    { C8_TOO_MANY_LABELS_EXCEPTION, C8_TOO_MANY_LABELS_EXCEPTION_MESSAGE },
    { C8_STACK_OVERFLOW_EXCEPTION, C8_STACK_OVERFLOW_EXCEPTION_MESSAGE },
    { C8_INVALID_ARGUMENT_EXCEPTION, C8_INVALID_ARGUMENT_EXCEPTION_MESSAGE },
    { C8_DUPLICATE_LABEL_EXCEPTION, C8_DUPLICATE_LABEL_EXCEPTION_MESSAGE },
    { C8_INVALID_SYMBOL_EXCEPTION, C8_INVALID_SYMBOL_EXCEPTION_MESSAGE },
    { C8_MEMORY_ALLOCATION_EXCEPTION, C8_MEMORY_ALLOCATION_EXCEPTION_MESSAGE },
    { C8_UNKNOWN_EXCEPTION, C8_UNKNOWN_EXCEPTION_MESSAGE },
    { C8_TOO_MANY_SYMBOLS_EXCEPTION, C8_TOO_MANY_SYMBOLS_EXCEPTION_MESSAGE },
    { C8_LOAD_FILE_FAILURE_EXCEPTION, C8_LOAD_FILE_FAILURE_EXCEPTION_MESSAGE },
    { C8_FILE_TOO_BIG_EXCEPTION, C8_FILE_TOO_BIG_EXCEPTION_MESSAGE },
    { C8_INVALID_COLOR_PALETTE_EXCEPTION, C8_INVALID_COLOR_PALETTE_EXCEPTION_MESSAGE },
    { C8_INVALID_QUIRK_EXCEPTION, C8_INVALID_QUIRK_EXCEPTION_MESSAGE },
    { C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION,
      C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION_MESSAGE },
    { C8_INVALID_FONT_EXCEPTION, C8_INVALID_FONT_EXCEPTION_MESSAGE },
    { C8_INVALID_CLOCK_SPEED_EXCEPTION, C8_INVALID_CLOCK_SPEED_EXCEPTION_MESSAGE },
    { C8_STACK_UNDERFLOW_EXCEPTION, C8_STACK_UNDERFLOW_EXCEPTION_MESSAGE },
};

char c8_exception[C8_EXCEPTION_MESSAGE_SIZE];

void c8_handle_exception(C8_ExceptionCode code) {
    for (size_t i = 0; i < sizeof(c8_exceptions) / sizeof(C8_Exception); i++) {
        if (c8_exceptions[i].code == code) {
            fprintf(stderr, "libc8: %s\n", c8_exceptions[i].message);
        }
    }

    fprintf(stderr, "libc8: %s\n", c8_exception);

#ifndef TEST
    exit(code);
#endif
}
