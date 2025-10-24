#include "unity.h"

#include "c8/encode.c"

#include "c8/private/util.h"
#include "c8/defs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BYTECODE_SIZE (C8_MEMSIZE - C8_PROG_START)
#define BUF_SIZE (BYTECODE_SIZE * C8_ENCODE_MAX_LINE_LENGTH)

char buf[BUF_SIZE];
uint8_t* bytecode;
int fmtCount;
int insCount;

symbol_list_t symbols;
label_list_t labels;

void setUp(void) {
    bytecode = calloc(BYTECODE_SIZE, 1);
    symbols.s = calloc(SYMBOL_CEILING, sizeof(symbol_t));
    symbols.ceil = SYMBOL_CEILING;
    labels.l = calloc(LABEL_CEILING, sizeof(label_t));
    labels.ceil = LABEL_CEILING;

    for (fmtCount = 0; formats[fmtCount].cmd != I_NULL; fmtCount++);
    for (insCount = 0; c8_instructionStrings[insCount] != NULL; insCount++);

    memset(bytecode, 0, BYTECODE_SIZE);
    memset(buf, 0, BUF_SIZE);

    memset(labels.l, 0, LABEL_CEILING * sizeof(label_t));
    labels.len = 0;
    labels.ceil = LABEL_CEILING;

    memset(symbols.s, 0, SYMBOL_CEILING * sizeof(symbol_t));
    symbols.len = 0;
    symbols.ceil = SYMBOL_CEILING;
}

void tearDown(void) {
    free(bytecode);
    free(symbols.s);
    free(labels.l);
}

void test_remove_comment_WhereStringHasNoComment(void) {
    const char* s = "String without a comment";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_STRING(s, remove_comment(buf));
}

void test_remove_comment_WhereStringHasCommentAtEnd(void) {
    const char* s = "String with a comment";
    sprintf(buf, "%s ; comment", s);
    TEST_ASSERT_EQUAL_STRING(s, remove_comment(buf));
}

void test_remove_comment_WhereStringIsOnlyComment(void) {
    const char* s = "; A comment";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(0, strlen(remove_comment(buf)));
}

void test_c8_encode_WhereStringIsOnlyComment(void) {
    const char* s = "; A comment";
    sprintf(buf, "%s\n", s);
    int r = c8_encode(buf, bytecode, 0);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(0, bytecode[0]);
}

void test_line_count_WhereStringHasOneLine(void) {
    const char* s = "ABCD";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(1, line_count(s));
}

void test_line_count_WhereStringHasMultipleLines(void) {
    const char* s = "ABCD\nEFGH\nIJKL\n";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(4, line_count(buf));
}

void test_parse_line_WhereLineIsEmpty(void) {
    const char* s = "";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(1, parse_line(buf, 1, &symbols, &labels));
}

void test_parse_line_WhereLineContainsWhitespace(void) {
    const char* s = "  ";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(1, parse_line(buf, 1, &symbols, &labels));
}

void test_parse_line_WhereLineContainsInstruction(void) {
    const char *s = "JP V0, $321";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(1, parse_line(buf, 1, &symbols, &labels));
}

void test_parse_line_WhereLineContainsDS(void) {
    const char *s = "Hello world";
    sprintf(buf, ".DS \"%s\"", s);

    int ret = parse_line(buf, 1, &symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, ret);
    for (int i = 0; s[i] != '\0'; i++) {
        TEST_ASSERT_EQUAL_INT(SYM_DB, symbols.s[i].type);
        TEST_ASSERT_EQUAL_INT((uint8_t)s[i], symbols.s[i].value);
        TEST_ASSERT_EQUAL_INT(1, symbols.s[i].ln);
    }
}

void test_parse_word_WhereWordIsLabelDefinition(void) {
    const char* s = "ldef";

    sprintf(buf, "%s:", s);
    labels.len = 1;
    sprintf(labels.l[0].identifier, "%s", s);
    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_LABEL_DEFINITION, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(0, symbols.s[0].value);
}

void test_parse_word_WhereWordIsInstruction(void) {
    int ins = 0;

    sprintf(buf, "%s", c8_instructionStrings[ins]);
    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_INSTRUCTION, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(ins, symbols.s[0].value);
}

void test_parse_word_WhereWordIsDB(void) {
    int v = 100;
    sprintf(buf, "%s", S_DB);
    sprintf(buf + 10, "%d", v);
    int r = parse_word(buf, buf + 10, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(SYM_DB, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v, symbols.s[0].value);
}

void test_parse_word_WhereWordIsDW(void) {
    int v = 500;
    sprintf(buf, "%s", S_DW);
    sprintf(buf + 10, "%d", v);
    int r = parse_word(buf, buf + 10, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(SYM_DW, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v, symbols.s[0].value);
}

void test_parse_word_WhereWordIsRegister(void) {
    int v = 12;
    sprintf(buf, "V%01x", v);
    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_V, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v, symbols.s[0].value);
}

void test_parse_word_WhereWordIsReservedIdentifier(void) {
    sprintf(buf, "%s", S_HF);
    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_HF, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(0, symbols.s[0].value);

    sprintf(buf, "%s", S_IP);
    r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_IP, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(0, symbols.s[0].value);
}

void test_parse_word_WhereWordIsInt(void) {
    int v4 = 8;
    int v8 = 128;
    int v12 = 512;

    sprintf(buf, "$%x", v4);
    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_INT4, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v4, symbols.s[0].value);

    memset(buf, 0, BUF_SIZE);

    sprintf(buf, "$%x", v8);
    r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_INT8, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v8, symbols.s[0].value);

    memset(buf, 0, BUF_SIZE);

    sprintf(buf, "$%x", v12);
    r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_INT12, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v12, symbols.s[0].value);
}

void test_parse_word_WhereWordIsLabel(void) {
    const char* l = "LABEL";
    labels.len = 2;
    sprintf(labels.l[0].identifier, "otherlabel");
    sprintf(labels.l[1].identifier, "%s", l);
    sprintf(buf, "%s", l);

    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(SYM_LABEL, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(1, symbols.s[0].value);
}

void test_parse_word_WhereWordIsInvalid(void) {
    const char* s = "Invalid";
    sprintf(buf, "%s", s);

    int r = parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(INVALID_SYMBOL_EXCEPTION, r);
    TEST_ASSERT_EQUAL_INT(0, symbols.len);
}

void test_tokenize_WhereStringIsOnlyWhitespace(void) {
    sprintf(buf, "\t\t      ");
    char* s[64];
    TEST_ASSERT_EQUAL_INT(1, tokenize(s, buf, " ", 10));
}
