#include "c8/common.h"
#include "c8/encode.h"
#include "c8/private/exception.h"
#include "c8/private/instruction.h"
#include "c8/private/symbol.h"

#include "unity.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BYTECODE_SIZE (C8_MEMSIZE - C8_PROG_START)
#define BUF_SIZE      (BYTECODE_SIZE * C8_ENCODE_MAX_LINE_LENGTH)

char          buf[BUF_SIZE];
uint8_t*      bytecode;
int           fmtCount;
int           insCount;

C8_SymbolList symbols;
C8_LabelList  labels;

extern int    c8_initialize_labels(C8_LabelList*);
extern int    c8_initialize_symbols(C8_SymbolList*);
extern int    c8_line_count(const char*);
extern int    c8_parse_line(char*, int, C8_SymbolList*, const C8_LabelList*);
extern int    c8_parse_word(char*, const char*, int, C8_Symbol*, const C8_LabelList*);
extern void   c8_put16(uint8_t*, uint16_t, int);
extern int    c8_tokenize(char**, char*, const char*, int);
extern int    c8_to_upper(char*);
extern char*  c8_remove_comma(char*);
extern int    c8_write(uint8_t*, C8_SymbolList*);

void          setUp(void) {
    bytecode     = calloc(BYTECODE_SIZE, 1);
    symbols.s    = calloc(C8_SYMBOL_CEILING, sizeof(C8_Symbol));
    symbols.ceil = C8_SYMBOL_CEILING;
    labels.l     = calloc(C8_LABEL_CEILING, sizeof(C8_Label));
    labels.ceil  = C8_LABEL_CEILING;

    for (fmtCount = 0; c8_formats[fmtCount].cmd != C8_I_NULL; fmtCount++)
        ;
    for (insCount = 0; c8_instructionStrings[insCount] != NULL; insCount++)
        ;

    memset(bytecode, 0, BYTECODE_SIZE);
    memset(buf, 0, BUF_SIZE);

    memset(labels.l, 0, C8_LABEL_CEILING * sizeof(C8_Label));
    labels.len  = 0;
    labels.ceil = C8_LABEL_CEILING;

    memset(symbols.s, 0, C8_SYMBOL_CEILING * sizeof(C8_Symbol));
    symbols.len  = 0;
    symbols.ceil = C8_SYMBOL_CEILING;
}

void tearDown(void) {
    free(bytecode);
    free(symbols.s);
    free(labels.l);
}

void test_c8_encode_WithEmptyInput(void) {
    int result = c8_encode("", bytecode, C8_ARG_VERBOSE);
    TEST_ASSERT_EQUAL_INT(C8_SYNTAX_ERROR_EXCEPTION, result);
}

void test_c8_encode_WithValidInput(void) {
    int result = c8_encode("ADD V0, x12\n", bytecode, C8_ARG_VERBOSE);
    TEST_ASSERT_EQUAL_INT(2, result);
    TEST_ASSERT_EQUAL_INT(0x70, bytecode[0]);
    TEST_ASSERT_EQUAL_INT(0x12, bytecode[1]);
}

void test_c8_encode_WithInvalidInput(void) {
    int result = c8_encode("ADD V0, x123456\n", bytecode, C8_ARG_VERBOSE);
    TEST_ASSERT_EQUAL_INT(C8_SYNTAX_ERROR_EXCEPTION, result);
}

void test_c8_remove_comment_WhereStringHasNoComment(void) {
    const char* s = "String without a comment";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_STRING(s, c8_remove_comment(buf));
}

void test_c8_remove_comment_WhereStringHasCommentAtEnd(void) {
    const char* s = "String with a comment";
    sprintf(buf, "%s ; comment", s);
    TEST_ASSERT_EQUAL_STRING(s, c8_remove_comment(buf));
}

void test_c8_remove_comment_WhereStringIsOnlyComment(void) {
    const char* s = "; A comment";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(0, strlen(c8_remove_comment(buf)));
}

void test_c8_encode_WhereStringIsOnlyComment(void) {
    const char* s = "; A comment";
    sprintf(buf, "%s\n", s);
    int r = c8_encode(buf, bytecode, 0);
    TEST_ASSERT_EQUAL_INT(C8_SYNTAX_ERROR_EXCEPTION, r);
    TEST_ASSERT_EQUAL_INT(0, bytecode[0]);
}

void test_c8_initialize_labels(void) {
    free(labels.l);
    TEST_ASSERT_EQUAL_INT(0, c8_initialize_labels(&labels));
    TEST_ASSERT_EQUAL_INT(0, labels.len);
    TEST_ASSERT_EQUAL_INT(C8_LABEL_CEILING, labels.ceil);
}

void test_c8_initialize_symbols(void) {
    free(symbols.s);
    TEST_ASSERT_EQUAL_INT(0, c8_initialize_symbols(&symbols));
    TEST_ASSERT_EQUAL_INT(0, symbols.len);
    TEST_ASSERT_EQUAL_INT(C8_SYMBOL_CEILING, symbols.ceil);
}

void test_c8_line_count_WhereStringHasOneLine(void) {
    const char* s = "ABCD";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(1, c8_line_count(s));
}

void test_c8_line_count_WhereStringHasMultipleLines(void) {
    const char* s = "ABCD\nEFGH\nIJKL\n";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(4, c8_line_count(buf));
}

void test_c8_parse_line_WhereLineIsEmpty(void) {
    const char* s = "";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(0, c8_parse_line(buf, 1, &symbols, &labels));
}

void test_c8_parse_line_WhereLineContainsWhitespace(void) {
    const char* s = "  ";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(0, c8_parse_line(buf, 1, &symbols, &labels));
}

void test_c8_parse_line_WhereLineContainsInstruction(void) {
    const char* s = "JP V0, $321";
    sprintf(buf, "%s", s);
    TEST_ASSERT_EQUAL_INT(0, c8_parse_line(buf, 1, &symbols, &labels));
}

void test_c8_parse_line_WhereLineContainsDS(void) {
    const char* s = "Hello world";
    sprintf(buf, ".DS \"%s\"", s);

    int ret = c8_parse_line(buf, 1, &symbols, &labels);

    TEST_ASSERT_EQUAL_INT(0, ret);
    for (int i = 0; s[i] != '\0'; i++) {
        TEST_ASSERT_EQUAL_INT(C8_SYM_DB, symbols.s[i].type);
        TEST_ASSERT_EQUAL_INT((uint8_t) s[i], symbols.s[i].value);
        TEST_ASSERT_EQUAL_INT(1, symbols.s[i].ln);
    }
}

void test_c8_parse_word_WhereWordIsLabelDefinition(void) {
    const char* s = "ldef";

    sprintf(buf, "%s:", s);
    labels.len = 1;
    sprintf(labels.l[0].identifier, "%s", s);
    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_LABEL_DEFINITION, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(0, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsInstruction(void) {
    int ins = 0;

    sprintf(buf, "%s", c8_instructionStrings[ins]);
    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_INSTRUCTION, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(ins, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsDB(void) {
    int v = 100;
    sprintf(buf, "%s", C8_S_DB);
    sprintf(buf + 10, "%d", v);
    int r = c8_parse_word(buf, buf + 10, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_DB, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsDW(void) {
    int v = 500;
    sprintf(buf, "%s", C8_S_DW);
    sprintf(buf + 10, "%d", v);
    int r = c8_parse_word(buf, buf + 10, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_DW, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsRegister(void) {
    int v = 12;
    sprintf(buf, "V%01x", v);
    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_V, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsReservedIdentifier(void) {
    sprintf(buf, "%s", C8_S_HF);
    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_HF, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(0, symbols.s[0].value);

    sprintf(buf, "%s", C8_S_IP);
    r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);

    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_IP, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(0, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsInt(void) {
    int v4  = 8;
    int v8  = 128;
    int v12 = 512;

    sprintf(buf, "$%x", v4);
    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_INT4, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v4, symbols.s[0].value);

    memset(buf, 0, BUF_SIZE);

    sprintf(buf, "$%x", v8);
    r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_INT8, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v8, symbols.s[0].value);

    memset(buf, 0, BUF_SIZE);

    sprintf(buf, "$%x", v12);
    r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_INT12, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(v12, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsLabel(void) {
    const char* l = "LABEL";
    labels.len    = 2;
    sprintf(labels.l[0].identifier, "otherlabel");
    sprintf(labels.l[1].identifier, "%s", l);
    sprintf(buf, "%s", l);

    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_LABEL, symbols.s[0].type);
    TEST_ASSERT_EQUAL_INT(1, symbols.s[0].value);
}

void test_c8_parse_word_WhereWordIsInvalid(void) {
    const char* s = "Invalid";
    sprintf(buf, "%s", s);

    int r = c8_parse_word(buf, NULL, 1, &symbols.s[0], &labels);
    TEST_ASSERT_EQUAL_INT(C8_SYNTAX_ERROR_EXCEPTION, r);
    TEST_ASSERT_EQUAL_INT(0, symbols.len);
}

void test_c8_put16(void) {
    c8_put16(bytecode, 0xABCD, 0);
    TEST_ASSERT_EQUAL_INT(0xAB, bytecode[0]);
    TEST_ASSERT_EQUAL_INT(0xCD, bytecode[1]);
}

void test_c8_tokenize_WhereStringIsOnlyWhitespace(void) {
    sprintf(buf, "\t\t      ");
    char* s[64];
    TEST_ASSERT_EQUAL_INT(1, c8_tokenize(s, buf, " ", 10));
}

void test_c8_tokenize_WhereStringContainsMultipleWords(void) {
    sprintf(buf, "hello, world!");
    char* s[64];
    TEST_ASSERT_EQUAL_INT(2, c8_tokenize(s, buf, " ", 10));
    TEST_ASSERT_EQUAL_STRING("hello,", s[0]);
    TEST_ASSERT_EQUAL_STRING("world!", s[1]);
}

void test_c8_remove_comma_WhereStringHasTrailingComma(void) {
    sprintf(buf, "hello,");
    c8_remove_comma(buf);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_c8_remove_comma_WhereStringHasNoTrailingComma(void) {
    sprintf(buf, "hello, world!");
    c8_remove_comma(buf);
    TEST_ASSERT_EQUAL_STRING("hello, world!", buf);
}

void test_c8_to_upper(void) {
    sprintf(buf, "hello, world!");
    c8_to_upper(buf);
    TEST_ASSERT_EQUAL_STRING("HELLO, WORLD!", buf);
}

void test_c8_write(void) {
    for (int i = 0; i < 64; i++) {
        switch (i % 3) {
        case 0:
            symbols.s[i].type  = C8_SYM_INSTRUCTION;
            symbols.s[i].value = C8_I_RET;
            break;
        case 1:
            symbols.s[i].type  = C8_SYM_DB;
            symbols.s[i].value = 16;
            break;
        case 2:
            symbols.s[i].type  = C8_SYM_DW;
            symbols.s[i].value = 512;
            break;
        }
    }
    TEST_ASSERT_EQUAL_INT(0, c8_write(bytecode, &symbols));

    int i           = 0;
    int bytecodeIdx = 0;
    while (i < symbols.len) {
        switch (symbols.s[i].type) {
        case C8_SYM_INSTRUCTION:
            TEST_ASSERT_EQUAL_INT(C8_I_RET, bytecode[bytecodeIdx++]);
            break;
        case C8_SYM_DB:
            TEST_ASSERT_EQUAL_INT(16, bytecode[bytecodeIdx++]);
            break;
        case C8_SYM_DW:
            TEST_ASSERT_EQUAL_INT(512, bytecode[bytecodeIdx]);
            bytecodeIdx += 2;
            break;
        default:
            break;
        }
        i++;
    }
}

void test_c8_write_WhereProgramIsTooLarge(void) {
    symbols.len  = 4096;
    symbols.ceil = 4096;
    free(symbols.s);
    symbols.s = malloc(sizeof(C8_Symbol) * 4096);
    for (int i = 0; i < 4096; i++) {
        switch (i % 3) {
        case 0:
            symbols.s[i].type  = C8_SYM_INSTRUCTION;
            symbols.s[i].value = C8_I_RET;
            break;
        case 1:
            symbols.s[i].type  = C8_SYM_DB;
            symbols.s[i].value = 16;
            break;
        case 2:
            symbols.s[i].type  = C8_SYM_DW;
            symbols.s[i].value = 512;
            break;
        }
    }

    TEST_ASSERT_EQUAL_INT(C8_SYNTAX_ERROR_EXCEPTION, c8_write(bytecode, &symbols));
}
