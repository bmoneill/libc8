#include "c8/font.h"

#include "unity.h"

#include <string.h>

#include "util.c"

C8   c8;

void test_c8_print_fonts(void) {
    c8_set_fonts(&c8, C8_SMALLFONT_OCTO, C8_BIGFONT_FISH);
    REDIRECT_STDOUT;
    c8_print_fonts(&c8);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("SFONT: octo\tBFONT: fish\n", stdio_buffer);
}
