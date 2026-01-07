
#include "unity.h"

#include "c8/private/debug.c"

#include <stdint.h>
#include <string.h>

#define BUFLEN 64

#define TEST_COMMAND(s, cmdid, argtype)                                                            \
    strcpy(buf, s);                                                                                \
    TEST_ASSERT_EQUAL_INT(1, c8_get_command(&cmd, buf));                                           \
    TEST_ASSERT_EQUAL_INT(cmdid, cmd.id);                                                          \
    TEST_ASSERT_EQUAL_INT(argtype, cmd.arg.type);

char       buf[BUFLEN];
C8_Command cmd;
C8         c8;

void       setUp(void) {
    memset(buf, 0, BUFLEN);
    memset(&cmd, 0, sizeof(C8_Command));
    memset(&c8, 0, sizeof(C8));
    c8.pc = 0x200;
}
void tearDown(void) {}

void test_c8_get_command_WhereCommandIsBreak(void) {
    TEST_COMMAND("break", C8_CMD_ADD_BREAKPOINT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsRMBreak(void) {
    TEST_COMMAND("rmbreak", C8_CMD_RM_BREAKPOINT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsContinue(void) {
    TEST_COMMAND("continue", C8_CMD_CONTINUE, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsNext(void) {
    TEST_COMMAND("next", C8_CMD_NEXT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsSet(void) {
    TEST_COMMAND("set PC 0x200", C8_CMD_SET, C8_ARG_PC);
    TEST_ASSERT_EQUAL_INT(0x200, cmd.setValue);
}

void test_c8_get_command_WhereCommandIsSet_WhereArgIsAddr(void) {
    int addr  = 0x20;
    int value = 0x12;

    TEST_COMMAND("set $20 0x12", C8_CMD_SET, C8_ARG_ADDR);
    TEST_ASSERT_EQUAL_INT(addr, cmd.arg.value.i);
    TEST_ASSERT_EQUAL_INT(value, cmd.setValue);
}

void test_c8_get_command_WhereCommandIsLoad(void) {
    TEST_COMMAND("load /path/to/file", C8_CMD_LOAD, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/file", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsSave(void) {
    TEST_COMMAND("save /path/to/file", C8_CMD_SAVE, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/file", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsLoadFlags(void) {
    TEST_COMMAND("loadflags /path/to/flags", C8_CMD_LOADFLAGS, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/flags", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsSaveFlags(void) {
    TEST_COMMAND("saveflags /path/to/flags", C8_CMD_SAVEFLAGS, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/flags", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsPrint_WithArgument(void) {
    TEST_COMMAND("print PC", C8_CMD_PRINT, C8_ARG_PC);
}

void test_c8_get_command_WhereCommandIsPrint_WithNoArguments(void) {
    TEST_COMMAND("print", C8_CMD_PRINT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsHelp(void) {
    TEST_COMMAND("help", C8_CMD_HELP, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsQuit(void) {
    TEST_COMMAND("quit", C8_CMD_QUIT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsInvalid(void) {
    const char* s = "invalid";
    strcpy(buf, s);
    TEST_ASSERT_EQUAL_INT(0, c8_get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(C8_CMD_NONE, cmd.id);
    TEST_ASSERT_EQUAL_INT(C8_ARG_NONE, cmd.arg.type);
}

void test_c8_run_command_WhereCommandIsAddBreakpoint_WithNoArgument(void) {
    cmd.id       = C8_CMD_ADD_BREAKPOINT;
    cmd.arg.type = C8_ARG_NONE;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(1, c8.breakpoints[0x200]);
}

void test_c8_run_command_WhereCommandIsAddBreakpoint_WithArgument(void) {
    int addr        = 0x246;
    cmd.id          = C8_CMD_ADD_BREAKPOINT;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = addr;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(1, c8.breakpoints[addr]);
}

void test_c8_run_command_WhereCommandIsRMBreakpoint_WithNoArgument(void) {
    c8.breakpoints[c8.pc] = 1;
    cmd.id                = C8_CMD_RM_BREAKPOINT;
    cmd.arg.type          = C8_ARG_NONE;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0, c8.breakpoints[c8.pc]);
}

void test_c8_run_command_WhereCommandIsRMBreakpoint_WithArgument(void) {
    int addr             = 0x123;
    c8.breakpoints[addr] = 1;
    cmd.id               = C8_CMD_RM_BREAKPOINT;
    cmd.arg.type         = C8_ARG_ADDR;
    cmd.arg.value.i      = addr;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0, c8.breakpoints[addr]);
}

void test_c8_run_command_WhereCommandIsContinue(void) {
    cmd.id = C8_CMD_CONTINUE;

    TEST_ASSERT_EQUAL_INT(C8_DEBUG_CONTINUE, c8_run_command(&c8, &cmd));
}

void test_c8_run_command_WhereCommandIsNext(void) {
    cmd.id = C8_CMD_NEXT;

    TEST_ASSERT_EQUAL_INT(C8_DEBUG_STEP, c8_run_command(&c8, &cmd));
}

void test_c8_run_command_WhereCommandIsQuit(void) {
    cmd.id = C8_CMD_QUIT;

    TEST_ASSERT_EQUAL_INT(C8_DEBUG_QUIT, c8_run_command(&c8, &cmd));
}

void test_c8_run_command_WhereCommandIsSet_WhereArgIsPC(void) {
    cmd.id       = C8_CMD_SET;
    cmd.arg.type = C8_ARG_PC;
    cmd.setValue = 0x300;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0x300, c8.pc);
}

void test_c8_run_command_WhereCommandIsSet_WhereArgIsADDR(void) {
    int addr        = 0x10;
    int value       = 0x23;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = addr;
    cmd.setValue    = value;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(value, c8.mem[addr]);
}
