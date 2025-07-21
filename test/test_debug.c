
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

void setUp() {
    memset(buf, 0, BUFLEN);
}
void tearDown() { }

void test_get_command_WhereCommandIsBreak(void) {
    cmd_t cmd;
    const char* s = "break";
    strcpy(buf, s);

    TEST_ASSERT_EQUAL_INT(1, get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(CMD_ADD_BREAKPOINT, cmd.id);
    TEST_ASSERT_EQUAL_INT(ARG_NONE, cmd.arg.type);
}

int main(void) {
    srand(time(NULL));

    UNITY_BEGIN();

    RUN_TEST(test_get_command_WhereCommandIsBreak);

    return UNITY_END();
}
