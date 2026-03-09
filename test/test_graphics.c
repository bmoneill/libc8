#include "c8/chip8.h"
#include "c8/graphics.h"

#include "unity.h"

#include <string.h>

C8   c8;

void setUp(void) { memset(&c8, 0, sizeof(C8)); }
void tearDown(void) {}

void test_c8_get_pixel_withLowDisplayMode(void) {
    c8.display.mode = C8_DISPLAYMODE_LOW;
    c8.display.p[0] = 1;
    TEST_ASSERT_EQUAL_INT(1, *c8_get_pixel(&c8.display, 0, 0));
}

void test_c8_get_pixel_withHighDisplayMode(void) {
    c8.display.mode = C8_DISPLAYMODE_HIGH;
    c8.display.p[0] = 1;
    TEST_ASSERT_EQUAL_INT(1, *c8_get_pixel(&c8.display, 0, 0));
}
