#include "c8/private/exception.c"

#include "unity.h"

#include "util.c"

void setUp(void) {}
void tearDown(void) {}

void test_c8_handle_exception(void) {
    sprintf(c8_exception, "Hello");
    REDIRECT_STDERR;
    c8_handle_exception(C8_AUDIO_EXCEPTION);
    RESTORE_STDERR;
    const char* expected = "libc8: " C8_AUDIO_EXCEPTION_MESSAGE "\nlibc8: Hello\n";
    TEST_ASSERT_EQUAL_STRING(expected, stdout_buffer);
}
