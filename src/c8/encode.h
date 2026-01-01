/**
 * @file c8/encode.h
 *
 * Base assembler code
 */

#ifndef C8_PARSE_H
#define C8_PARSE_H

#include <stdint.h>

#define C8_ARG_VERBOSE 1

#define C8_ENCODE_MAX_LINE_LENGTH 100
#define C8_ENCODE_MAX_WORDS       100
#define C8_ENCODE_MAX_LINES       500

extern char** c8_lines;
extern char** c8_linesUnformatted;
extern int    c8_lineCount;

int           c8_encode(const char*, uint8_t*, int);
char*         c8_remove_comment(char*);

#endif
