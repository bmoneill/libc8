/**
 * @file c8/encode.h
 *
 * Base assembler code
 */

#ifndef C8_PARSE_H
#define C8_PARSE_H

#include <stdint.h>

/**
 * @brief Print verbose output
 */
#define C8_ARG_VERBOSE 1

/**
 * @brief Maximum line length
 */
#define C8_ENCODE_MAX_LINE_LENGTH 100

/**
 * @brief Maximum number of words per line
 */
#define C8_ENCODE_MAX_WORDS 100

/**
 * @brief Maximum number of lines
 */
#define C8_ENCODE_MAX_LINES 500

/**
 * @brief Source code (will be manipulated when encoding)
 */
extern char** c8_lines;

/**
 * @brief Source code (will not be manipulated when encoding)
 */
extern char** c8_linesUnformatted;

/**
 * @brief Source code line count.
 */
extern int c8_lineCount;

int        c8_encode(const char*, uint8_t*, int);
char*      c8_remove_comment(char*);

#endif
