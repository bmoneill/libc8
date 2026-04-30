// Common definitions for test_debug.c and test_debug_linux.c
#ifndef TEST_DEBUG_H
#define TEST_DEBUG_H

#include "c8/private/debug.h"

#include "c8/chip8.h"
#include "c8/font.h"
#include "c8/graphics.h"
#include "c8/private/exception.h"
#include "unity.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_COMMAND(s, cmdid, argtype)                                                            \
    strcpy(buf, s);                                                                                \
    TEST_ASSERT_EQUAL_INT(0, c8_get_command(&cmd, buf));                                           \
    TEST_ASSERT_EQUAL_INT(cmdid, cmd.id);                                                          \
    TEST_ASSERT_EQUAL_INT(argtype, cmd.arg.type);

#define DEBUG_BUFFER_LENGTH 128

char                 buf[DEBUG_BUFFER_LENGTH];
C8_Command           cmd;
C8                   c8;

extern int           c8_get_command(C8_Command*, char*);
extern int           c8_load_file_arg(C8_Command*, char*);
extern int           c8_load_flags(C8*, const char*);
extern int           c8_load_state(C8*, const char*);
extern int           c8_parse_arg(C8_Command*, char*);
extern void          c8_print_help(void);
extern void          c8_print_r_registers(const C8*);
extern void          c8_print_stack(const C8*);
extern void          c8_print_v_registers(const C8*);
extern void          c8_print_quirks(int);
extern void          c8_print_value(C8*, const C8_Command*);
extern int           c8_run_command(C8*, const C8_Command*);
extern int           c8_save_flags(const C8*, const char*);
extern int           c8_save_state(const C8*, const char*);
extern int           c8_set_value(C8*, const C8_Command*);

extern const uint8_t c8_smallFonts[5][80];
extern const uint8_t c8_bigFonts[3][160];

#endif
