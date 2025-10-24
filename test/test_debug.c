
#include "unity.h"

#include "c8/defs.h"
#include "c8/private/debug.c"
#include "c8/private/exception.h"
#include "c8/private/util.h"

#include <stdint.h>
#include <string.h>

#define BUFLEN 64

char  buf[BUFLEN];
cmd_t cmd;
c8_t  c8;

void  setUp(void) {
    memset(buf, 0, BUFLEN);
    memset(&cmd, 0, sizeof(cmd_t));
    memset(&c8, 0, sizeof(c8_t));
    c8.pc = 0x200;
}
void tearDown(void) {}

void test_get_command_WhereCommandIsBreak(void) {
    const char* s = "break";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_ADD_BREAKPOINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsRMBreak(void) {
    const char* s = "rmbreak";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_RM_BREAKPOINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsContinue(void) {
    const char* s = "continue";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_CONTINUE, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsNext(void) {
    const char* s = "next";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_NEXT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsSet(void) {
    const char* s = "set PC 0x200";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_SET, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_PC, cmd.arg.type);
    TEST_ASSERT_EQUAL_INT(0x200, cmd.setValue);
}

void test_get_command_WhereCommandIsSet_WhereArgIsAddr(void) {
    int addr  = 0x20;
    int value = 0x12;
    sprintf(buf, "set $%03x %d", addr, value);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_SET, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_ADDR, cmd.arg.type);
    TEST_ASSERT_EQUAL_INT(addr, cmd.arg.value.i);
    TEST_ASSERT_EQUAL_INT(value, cmd.setValue);
}

void test_get_command_WhereCommandIsLoad(void) {
    const char* s = "load /path/to/file";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_LOAD, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_FILE, cmd.arg.type);
    TEST_ASSERT_EQUAL_STRING("/path/to/file", cmd.arg.value.s);
}

void test_get_command_WhereCommandIsSave(void) {
    const char* s = "save /path/to/file";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_SAVE, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_FILE, cmd.arg.type);
    TEST_ASSERT_EQUAL_STRING("/path/to/file", cmd.arg.value.s);
}

void test_get_command_WhereCommandIsLoadFlags(void) {
    const char* s = "loadflags /path/to/flags";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_LOADFLAGS, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_FILE, cmd.arg.type);
    TEST_ASSERT_EQUAL_STRING("/path/to/flags", cmd.arg.value.s);
}

void test_get_command_WhereCommandIsSaveFlags(void) {
    const char* s = "saveflags /path/to/flags";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_SAVEFLAGS, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_FILE, cmd.arg.type);
    TEST_ASSERT_EQUAL_STRING("/path/to/flags", cmd.arg.value.s);
}

void test_get_command_WhereCommandIsPrint_WithArgument(void) {
    const char* s = "print PC";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_PRINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_PC, cmd.arg.type);
}

void test_get_command_WhereCommandIsPrint_WithNoArguments(void) {
    const char* s = "print";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_PRINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsHelp(void) {
    const char* s = "help";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_HELP, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsQuit(void) {
    const char* s = "quit";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_QUIT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_get_command_WhereCommandIsInvalid(void) {
    const char* s = "invalid";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(0, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_NONE, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_run_command_WhereCommandIsAddBreakpoint_WithNoArgument(void) {
    cmd.id       = CMD_ADD_BREAKPOINT;
    cmd.arg.type = ARG_NONE;

    TEST_ASSERT_EQUAL_INT(0, run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(1, c8.breakpoints[0x200]);
}

void test_run_command_WhereCommandIsAddBreakpoint_WithArgument(void) {
    int addr        = 0x246;
    cmd.id          = CMD_ADD_BREAKPOINT;
    cmd.arg.type    = ARG_ADDR;
    cmd.arg.value.i = addr;

    TEST_ASSERT_EQUAL_INT(0, run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(1, c8.breakpoints[addr]);
}

void test_run_command_WhereCommandIsRMBreakpoint_WithNoArgument(void) {
    c8.breakpoints[c8.pc] = 1;
    cmd.id                = CMD_RM_BREAKPOINT;
    cmd.arg.type          = ARG_NONE;

    TEST_ASSERT_EQUAL_INT(0, run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0, c8.breakpoints[c8.pc]);
}

void test_run_command_WhereCommandIsRMBreakpoint_WithArgument(void) {
    int addr             = 0x123;
    c8.breakpoints[addr] = 1;
    cmd.id               = CMD_RM_BREAKPOINT;
    cmd.arg.type         = ARG_ADDR;
    cmd.arg.value.i      = addr;

    TEST_ASSERT_EQUAL_INT(0, run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0, c8.breakpoints[addr]);
}

void test_run_command_WhereCommandIsContinue(void) {
    cmd.id = CMD_CONTINUE;

    TEST_ASSERT_EQUAL_INT(DEBUG_CONTINUE, run_command(&c8, &cmd));
}

void test_run_command_WhereCommandIsNext(void) {
    cmd.id = CMD_NEXT;

    TEST_ASSERT_EQUAL_INT(DEBUG_STEP, run_command(&c8, &cmd));
}

void test_run_command_WhereCommandIsQuit(void) {
    cmd.id = CMD_QUIT;

    TEST_ASSERT_EQUAL_INT(DEBUG_QUIT, run_command(&c8, &cmd));
}

void test_run_command_WhereCommandIsSet_WhereArgIsPC(void) {
    cmd.id       = CMD_SET;
    cmd.arg.type = ARG_PC;
    cmd.setValue = 0x300;

    TEST_ASSERT_EQUAL_INT(0, run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0x300, c8.pc);
}

void test_run_command_WhereCommandIsSet_WhereArgIsADDR(void) {
    int addr        = 0x10;
    int value       = 0x23;
    cmd.id          = CMD_SET;
    cmd.arg.type    = ARG_ADDR;
    cmd.arg.value.i = addr;
    cmd.setValue    = value;

    TEST_ASSERT_EQUAL_INT(0, run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(value, c8.mem[addr]);
}
