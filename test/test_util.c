#include "c8/common.h"
#include "c8/private/exception.c"
#include "c8/private/util.c"
#include "unity.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 64

char buf[BUF_SIZE];

void setUp(void) {
    srand(time(NULL));
    memset(buf, 0, BUF_SIZE);
}

void tearDown(void) {}

void test_c8_hex_to_int_WhereStringIsLowercase(void) {
    int  n = (rand() % 6) + 10;
    char c = (n + 0x61) - 10;
    TEST_ASSERT_EQUAL_INT(n, c8_hex_to_int(c));
}

void test_c8_hex_to_int_WhereStringIsUppercase(void) {
    int  n = (rand() % 6) + 10;
    char c = (n + 0x41) - 10;
    TEST_ASSERT_EQUAL_INT(n, c8_hex_to_int(c));
}

void test_c8_hex_to_int_WhereStringIsDecimal(void) {
    int  n = rand() % 10;
    char c = n + 0x30;
    TEST_ASSERT_EQUAL_INT(n, c8_hex_to_int(c));
}

void test_c8_parse_int_WhereStringIsDecimal(void) {
    int n = rand();
    sprintf(buf, "%d", n);
    TEST_ASSERT_EQUAL_INT(n, c8_parse_int(buf));
}

void test_c8_parse_int_WhereStringIsHexWithDollarPrefix(void) {
    int n = rand();
    sprintf(buf, "$%x", n);
    TEST_ASSERT_EQUAL_INT(n, c8_parse_int(buf));
}

void test_c8_parse_int_WhereStringIsHexWithXPrefix(void) {
    int n = rand();
    sprintf(buf, "x%x", n);
    TEST_ASSERT_EQUAL_INT(n, c8_parse_int(buf));
}

void test_c8_parse_int_WhereStringIsHexWithVPrefix(void) {
    int n = rand();
    sprintf(buf, "V%x", n);
    TEST_ASSERT_EQUAL_INT(n, c8_parse_int(buf));
}

void test_c8_parse_int_WhereStringIsEmpty(void) { TEST_ASSERT_EQUAL_INT(-1, c8_parse_int(buf)); }

void test_c8_parse_int_WhereIntIsZero(void) {
    int n = 0;
    sprintf(buf, "%d", n);
    TEST_ASSERT_EQUAL_INT(n, c8_parse_int(buf));
}

void test_c8_parse_int_WhereStringDoesNotContainInt(void) {
    sprintf(buf, "A string without an integer");
    TEST_ASSERT_EQUAL_INT(-1, c8_parse_int(buf));
}

void test_c8_trim_WhereStringIsOnlyWhitespace(void) {
    sprintf(buf, "      \t\t");
    TEST_ASSERT_EQUAL_INT(0, strlen(c8_trim(buf)));
}

void test_c8_trim_WhereStringHasLeadingWhitespace(void) {
    const char* content = "Hello there";
    sprintf(buf, "        \t\t  %s", content);
    TEST_ASSERT_EQUAL_STRING(content, c8_trim(buf));
}

void test_c8_trim_WhereStringHasTrailingWhitespace(void) {
    const char* content = "Hello there";
    sprintf(buf, "%s        \t\t\t", content);
    TEST_ASSERT_EQUAL_STRING(content, c8_trim(buf));
}

void test_c8_trim_leading_WhereStringHasLeadingAndTrailingWhitespace(void) {
    const char* content = "Hello there";
    sprintf(buf, "         \t\t       %s   \t\t\t", content);
    TEST_ASSERT_EQUAL_STRING(content, c8_trim(buf));
}

void test_c8_trim_WhereStringHasNoWhitespace(void) {
    const char* content = "Hello there";
    sprintf(buf, "%s", content);
    TEST_ASSERT_EQUAL_STRING(content, c8_trim(buf));
}
