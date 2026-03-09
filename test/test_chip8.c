#include "c8/chip8.h"
#include "c8/private/exception.h"
#include "util.c"

#include "unity.h"

#include <stdlib.h>
#include <string.h>

C8   c8;
char buf[1024];

void setUp(void) { memset(&c8, 0, sizeof(c8)); }

void tearDown(void) { memset(c8_exception, 0, 8192); }

void test_c8_init_WithValidPath(void) {
    char* path      = get_path("1dcell.ch8");
    C8*   c8_allocd = c8_init(path, 0);
    TEST_ASSERT_NOT_NULL(c8_allocd);
    c8_deinit(c8_allocd);
}

void test_c8_init_WithInvalidPath(void) {
    C8* c8_allocd = c8_init("non_existent.ch8", 0);
    TEST_ASSERT_NULL(c8_allocd);
}

void test_c8_init_WithNullPath(void) {
    C8* c8_allocd = c8_init(NULL, 0);
    TEST_ASSERT_NOT_NULL(c8_allocd);
}

void test_c8_load_palette_s_WithValidColorPalette(void) {
    sprintf(buf, "0xABCDEF,0x123456");
    int result = c8_load_palette_s(&c8, buf);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(0xABCDEF, c8.colors[0]);
    TEST_ASSERT_EQUAL_INT(0x123456, c8.colors[1]);
}

void test_c8_load_palette_s_WithInvalidColorPalette(void) {
    sprintf(buf, "blabla,123");
    int result = c8_load_palette_s(&c8, buf);
    TEST_ASSERT_EQUAL_INT(C8_INVALID_PARAMETER_EXCEPTION, result);
}

void test_c8_load_palette_f_WithValidColorPalette(void) {
    char* path   = get_path("colors.txt");
    int   result = c8_load_palette_f(&c8, path);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(0xABCDEF, c8.colors[0]);
    TEST_ASSERT_EQUAL_INT(0x123456, c8.colors[1]);
}

void test_c8_load_palette_f_WithInvalidColorPalette(void) {
    char* path   = get_path("colors-invalid.txt");
    int   result = c8_load_palette_f(&c8, path);
    TEST_ASSERT_EQUAL_INT(C8_INVALID_PARAMETER_EXCEPTION, result);
}

void test_c8_load_quirks_WithValidQuirks(void) {
    int result = c8_load_quirks(&c8, "bdj");
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(C8_FLAG_QUIRK_BITWISE, c8.flags & C8_FLAG_QUIRK_BITWISE);
    TEST_ASSERT_EQUAL_INT(C8_FLAG_QUIRK_DRAW, c8.flags & C8_FLAG_QUIRK_DRAW);
    TEST_ASSERT_EQUAL_INT(C8_FLAG_QUIRK_JUMP, c8.flags & C8_FLAG_QUIRK_JUMP);
}

void test_c8_load_quirks_WithInvalidQuirks(void) {
    int result = c8_load_quirks(&c8, "bdjx");
    TEST_ASSERT_EQUAL_INT(C8_INVALID_PARAMETER_EXCEPTION, result);
}

void test_c8_load_rom_WhereFileIsValid(void) {
    char* path   = get_path("1dcell.ch8");
    int   result = c8_load_rom(&c8, path);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_c8_load_rom_WhereFileIsNotValidROM(void) {
    char* path   = get_path("colors.txt");
    int   result = c8_load_rom(&c8, path);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_c8_load_rom_WhereFileDoesNotExist(void) {
    char* path   = get_path("non_existent.txt");
    int   result = c8_load_rom(&c8, path);
    TEST_ASSERT_EQUAL_INT(C8_IO_EXCEPTION, result);
}

void test_c8_validate_WithValidC8(void) {
    C8* c8_allocd = c8_init(NULL, 0);
    TEST_ASSERT_EQUAL_INT(0, c8_validate(c8_allocd));
    free(c8_allocd);
}

void test_c8_validate_WithInvalidPC(void) {
    C8* c8_allocd = c8_init(NULL, 0);
    c8_allocd->pc = 0xFFFF;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));
    free(c8_allocd);
}

void test_c8_validate_WithInvalidCS(void) {
    C8* c8_allocd = c8_init(NULL, 0);

    c8_allocd->cs = 0;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    c8_allocd->cs = -1;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    free(c8_allocd);
}

void test_c8_validate_WithInvalidVK(void) {
    C8* c8_allocd = c8_init(NULL, 0);

    c8_allocd->VK = 16;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    c8_allocd->VK = -1;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    free(c8_allocd);
}

void test_c8_validate_WithInvalidMode(void) {
    C8* c8_allocd   = c8_init(NULL, 0);

    c8_allocd->mode = 4;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    c8_allocd->mode = -1;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    free(c8_allocd);
}

void test_c8_validate_WithInvalidDisplayMode(void) {
    C8* c8_allocd           = c8_init(NULL, 0);

    c8_allocd->display.mode = 2;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    c8_allocd->mode = -1;
    TEST_ASSERT_EQUAL_INT(C8_INVALID_STATE_EXCEPTION, c8_validate(c8_allocd));

    free(c8_allocd);
}

void test_c8_version(void) {
    // TODO
    TEST_ASSERT_EQUAL_STRING(C8_VERSION, c8_version());
}
