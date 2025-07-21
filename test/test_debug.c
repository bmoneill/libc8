
#include "unity.h"

#include "c8/private/debug.c"
#include "c8/private/exception.h"
#include "c8/private/util.h"
#include "c8/defs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFLEN 64

char buf[BUFLEN];
cmd_t cmd;

void setUp() {
    memset(buf, 0, BUFLEN);
    memset(&cmd, 0, sizeof(cmd_t));
}
void tearDown() { }

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

void test_command_WhereCommandIsPrint(void) {
    const char* s = "print PC";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_PRINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_PC, cmd.arg.type);
}

void test_command_WhereCommandIsPrint_WithNoArguments(void) {
    const char* s = "print";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_PRINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_command_WhereCommandIsHelp(void) {
    const char* s = "help";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_HELP, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

void test_command_WhereCommandIsQuit(void) {
    const char* s = "quit";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_QUIT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

int main(void) {
    srand(time(NULL));

    UNITY_BEGIN();

    RUN_TEST(test_get_command_WhereCommandIsBreak);
    RUN_TEST(test_get_command_WhereCommandIsRMBreak);
    RUN_TEST(test_get_command_WhereCommandIsContinue);
    RUN_TEST(test_get_command_WhereCommandIsNext);
    RUN_TEST(test_get_command_WhereCommandIsSet);
    RUN_TEST(test_get_command_WhereCommandIsLoad);
    RUN_TEST(test_get_command_WhereCommandIsSave);
    RUN_TEST(test_get_command_WhereCommandIsLoadFlags);
    RUN_TEST(test_get_command_WhereCommandIsSaveFlags);
    RUN_TEST(test_command_WhereCommandIsPrint);
    RUN_TEST(test_command_WhereCommandIsPrint_WithNoArguments);
    RUN_TEST(test_command_WhereCommandIsHelp);
    RUN_TEST(test_command_WhereCommandIsQuit);

    return UNITY_END();
}
