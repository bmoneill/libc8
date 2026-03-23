#include "c8/font.h"

#include "unity.h"

#include <string.h>

#include "util.c"

C8                   c8;

extern const uint8_t c8_smallFonts[][80];
extern const uint8_t c8_bigFonts[][160];

void                 setUp(void) { memset(&c8, 0, sizeof(C8)); }
void                 tearDown(void) {}

void                 test_c8_set_fonts(void) {
    c8_set_fonts(&c8, C8_SMALLFONT_FISH, C8_BIGFONT_FISH);
    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_smallFonts[C8_SMALLFONT_FISH][i], c8.mem[C8_FONT_START + i]);
    }
    for (int i = 0; i < 160; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_bigFonts[C8_BIGFONT_FISH][i], c8.mem[C8_HIGH_FONT_START + i]);
    }

    c8_set_fonts(&c8, C8_SMALLFONT_VIP, -1);
    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_smallFonts[C8_SMALLFONT_VIP][i], c8.mem[C8_FONT_START + i]);
    }

    c8_set_fonts(&c8, -1, C8_BIGFONT_SCHIP);
    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_bigFonts[C8_BIGFONT_SCHIP][i], c8.mem[C8_HIGH_FONT_START + i]);
    }
}

void test_c8_set_fonts_s(void) {
    char fontStr[16] = "octo,fish\0";
    c8_set_fonts_s(&c8, fontStr);
    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_smallFonts[C8_SMALLFONT_OCTO][i], c8.mem[C8_FONT_START + i]);
    }
    for (int i = 0; i < 160; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_bigFonts[C8_BIGFONT_FISH][i], c8.mem[C8_HIGH_FONT_START + i]);
    }
}

void test_c8_set_small_font(void) {
    int result = c8_set_small_font(&c8, "octo");
    TEST_ASSERT_EQUAL_INT(0, result);
    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_smallFonts[C8_SMALLFONT_OCTO][i], c8.mem[C8_FONT_START + i]);
    }

    result = c8_set_small_font(&c8, "invalid");
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_c8_set_big_font(void) {
    int result = c8_set_big_font(&c8, "octo");
    TEST_ASSERT_EQUAL_INT(0, result);
    for (int i = 0; i < 160; i++) {
        TEST_ASSERT_EQUAL_UINT8(c8_bigFonts[C8_BIGFONT_OCTO][i], c8.mem[C8_HIGH_FONT_START + i]);
    }

    result = c8_set_big_font(&c8, "invalid");
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_c8_print_fonts(void) {
    c8_set_fonts(&c8, C8_SMALLFONT_OCTO, C8_BIGFONT_FISH);
    REDIRECT_STDOUT;
    c8_print_fonts(&c8);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("SFONT: octo\tBFONT: fish\n", stdio_buffer);
}
