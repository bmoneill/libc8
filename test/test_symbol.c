#include "unity.h"

#include "c8/private/symbol.c"
#include "c8/private/exception.h"
#include "c8/private/util.h"
#include "c8/encode.h"
#include "c8/defs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_COUNT 10
#define MAX_LINE_LEN 50

instruction_t ins;
symbol_list_t symbols;
label_list_t labels;
const char* empty = "\0";
int fc = 0;
char* line0;

void setUp(void) {
    memset(&ins, 0, sizeof(instruction_t));
    memset(line0, 0, MAX_LINE_LEN * MAX_LINE_COUNT);

    memset(labels.l, 0, LABEL_CEILING * sizeof(label_t));
    labels.len = 0;
    labels.ceil = LABEL_CEILING;

    memset(symbols.s, 0, SYMBOL_CEILING * sizeof(symbol_t));
    symbols.len = 0;
    symbols.ceil = SYMBOL_CEILING;
}

void tearDown(void) { }

void test_is_comment_WhereCommentIsAtEndOfString(void) {
    const char* s = "Hello ; This is a comment";
    TEST_ASSERT_EQUAL_INT(0, is_comment(s));
}

void test_is_comment_WhereCommentIsEntireString(void) {
    const char* s = "; This is a comment";
    TEST_ASSERT_EQUAL_INT(1, is_comment(s));
}

void test_is_comment_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(0, is_comment(empty));
}

void test_is_comment_WhereNoCommentIsInString(void) {
    const char* s = "This is not a comment";
    TEST_ASSERT_EQUAL_INT(0, is_comment(s));
}

void test_is_db_WhereStringIsDB(void) {
    TEST_ASSERT_EQUAL_INT(1, is_db(S_DB));
}

void test_is_db_WhereStringIsNotDB(void) {
    TEST_ASSERT_EQUAL_INT(0, is_db(S_DW));
}

void test_is_db_WhereStringContainsDB(void) {
    const char* s = "Foo .DB";
    TEST_ASSERT_EQUAL_INT(0, is_db(s));
}

void test_is_db_WithTrailingChars(void) {
    const char* s = ".DB foo";
    TEST_ASSERT_EQUAL_INT(0, is_db(s));
}

void test_is_db_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(0, is_db(empty));
}

void test_is_dw_WhereStringIsDW(void) {
    TEST_ASSERT_EQUAL_INT(1, is_dw(S_DW));
}

void test_is_dw_WhereStringIsNotDW(void) {
    TEST_ASSERT_EQUAL_INT(0, is_dw(S_DB));
}

void test_is_dw_WhereStringContainsDW(void) {
    const char* s = "Foo .DW";
    TEST_ASSERT_EQUAL_INT(0, is_db(s));
}

void test_is_dw_WithTrailingChars(void) {
    const char* s = ".DW foo";
    TEST_ASSERT_EQUAL_INT(0, is_dw(s));
}

void test_is_dw_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(0, is_dw(empty));
}

void test_is_instruction_WhereStringIsInstruction(void) {
    int ic = 0;
    for (ic = 0; c8_instructionStrings[ic] != NULL; ic++);

    int i = rand() % ic;

    char s[16];
    strcpy(s, c8_instructionStrings[i]);

    TEST_ASSERT_EQUAL_INT(i, is_instruction(s));
}

void test_is_instruction_WhereStringIsNotInstruction(void) {
    const char* s = "Not an instruction";

    TEST_ASSERT_EQUAL_INT(-1, is_instruction(s));
}

void test_is_instruction_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, is_instruction(empty));
}

void test_is_label_definition_WhereStringIsLabelDefinition(void) {
    const char* s = "L:";

    TEST_ASSERT_EQUAL_INT(1, is_label_definition(s));
}

void test_is_label_definition_WhereStringIsNotLabelDefinition(void) {
    const char* s = "L";

    TEST_ASSERT_EQUAL_INT(0, is_label_definition(s));
}

void test_is_label_definition_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(0, is_label_definition(empty));
}

void test_is_label_WhereStringIsLabel(void) {
    const char* s = "L";

    labels.len = 3;
    labels.l[0].byte = rand();
    strcpy(labels.l[0].identifier, "LABEL");
    labels.l[1].byte = rand();
    strcpy(labels.l[1].identifier, "ANOTHERLABEL");
    labels.l[2].byte = rand();
    strcpy(labels.l[2].identifier, "L");

    TEST_ASSERT_EQUAL_INT(2, is_label(s, &labels));
}

void test_is_label_WhereStringIsNotLabel(void) {
    const char* s = "L";

    labels.len = 3;
    labels.l[0].byte = rand();
    strcpy(labels.l[0].identifier, "LABEL");
    labels.l[1].byte = rand();
    strcpy(labels.l[1].identifier, "ANOTHERLABEL");
    labels.l[2].byte = rand();
    strcpy(labels.l[2].identifier, "L");

    TEST_ASSERT_EQUAL_INT(-1, is_label("LABEL3", &labels));
}

void test_is_label_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, is_label(empty, &labels));
}

void test_is_register_WhereStringIsRegister_WhereRegisterIsUppercase(void) {
    const char* s = "V1";
    TEST_ASSERT_EQUAL_INT(1, is_register(s));
}

void test_is_register_WhereStringIsRegister_RegisterIsLowercase(void) {
    const char* s = "vf";
    TEST_ASSERT_EQUAL_INT(0xF, is_register(s));
}

void test_is_register_WhereStringIsNotRegister(void) {
    const char* s = "x4";
    TEST_ASSERT_EQUAL_INT(-1, is_register(s));
}

void test_is_register_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, is_register(empty));
}

void test_is_reserved_identifier_WhereStringIsReservedIdentifier(void) {
    int ic = 0;
    for (ic = 0; c8_identifierStrings[ic] != NULL; ic++);

    int ident = rand() % ic;

    char s[16];
    strcpy(s, c8_identifierStrings[ident]);

    TEST_ASSERT_EQUAL_INT(ident, is_reserved_identifier(s));
}

void test_is_reserved_identifier_WhereStringIsNotReservedIdentifier(void) {
    const char* s = "Not reserved";

    TEST_ASSERT_EQUAL_INT(-1, is_reserved_identifier(s));
}

void test_is_reserved_identifier_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, is_reserved_identifier(empty));
}

void test_next_symbol_WhereSymbolListIsEmpty(void) {
    memset(symbols.s, 0, SYMBOL_CEILING);
    symbols.len = 0;
    symbols.ceil = SYMBOL_CEILING;

    TEST_ASSERT_EQUAL_PTR(&symbols.s[0], next_symbol(&symbols));
}

void test_next_symbol_WhereSymbolListIsNotEmptyOrFull(void) {
    memset(symbols.s, 0, SYMBOL_CEILING);

    symbols.len = 2;
    symbols.ceil = SYMBOL_CEILING;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_IP;

    TEST_ASSERT_EQUAL_PTR(&symbols.s[2], next_symbol(&symbols));
}

void test_next_symbol_WhereSymbolListIsFull(void) {
    memset(symbols.s, SYM_INSTRUCTION, SYMBOL_CEILING);

    symbols.len = SYMBOL_CEILING;
    symbols.ceil = SYMBOL_CEILING;

    symbol_t* symbol = next_symbol(&symbols);

    TEST_ASSERT_EQUAL_INT(SYMBOL_CEILING + 1, symbols.len);
    TEST_ASSERT_EQUAL_INT(SYMBOL_CEILING * 2, symbols.ceil);
    TEST_ASSERT_EQUAL_PTR(&symbols.s[SYMBOL_CEILING], symbol);
}

void test_populate_labels_WhereLinesIsEmpty(void) {
    int r = populate_labels(&labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0, labels.len);
    TEST_ASSERT_EQUAL_STRING("\0", labels.l[0].identifier);
}

void test_populate_labels_WhereLabelListIsEmpty(void) {
    sprintf(c8_lines[0], "%s", "ADD V4, V5");
    sprintf(c8_lines[1], "%s", "\0");
    sprintf(c8_lines[2], "%s", "CLS");
    sprintf(c8_lines[3], "%s", "RET");
    sprintf(c8_lines[4], "%s", "SE V1, $55");

    int r = populate_labels(&labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0, labels.len);
    TEST_ASSERT_EQUAL_STRING("\0", labels.l[0].identifier);
}

void test_populate_labels_WhereLinesHasMultipleLabelDefinitions(void) {
    sprintf(c8_lines[0], "%s", "ADD V4, V5");
    sprintf(c8_lines[1], "%s", "label:");
    sprintf(c8_lines[2], "%s", "RET");
    sprintf(c8_lines[3], "%s", "otherlabel:");
    sprintf(c8_lines[4], "%s", "SE V1, $55");

    int r = populate_labels(&labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(2, labels.len);
    TEST_ASSERT_EQUAL_STRING("label", labels.l[0].identifier);
    TEST_ASSERT_EQUAL_STRING("otherlabel", labels.l[1].identifier);
}

void test_populate_labels_WhereLinesHasDuplicateLabelDefinitions(void) {
    sprintf(c8_lines[0], "%s", "ADD V4, V5");
    sprintf(c8_lines[1], "%s", "label:");
    sprintf(c8_lines[2], "%s", "RET");
    sprintf(c8_lines[3], "%s", "label:");
    sprintf(c8_lines[4], "%s", "SE V1, $55");
    c8_line_count = 5;

    int r = populate_labels(&labels);

    TEST_ASSERT_EQUAL_INT(DUPLICATE_LABEL_EXCEPTION, r);
}

void test_resolve_labels_WhereLabelListHasOneLabel_WhereSymbolListHasLabelDefinition(void) {

    symbols.len = 3;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_LABEL;
    symbols.s[1].value = 0;
    symbols.s[2].type = SYM_LABEL_DEFINITION;
    symbols.s[2].value = 0;
    symbols.s[3].type = SYM_INSTRUCTION;

    labels.len = 1;
    sprintf(labels.l[0].identifier, "%s", "LABEL");

    int r = resolve_labels(&symbols, &labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0x202, labels.l[0].byte);
}

void test_resolve_labels_WhereLabelListHasOneLabel_WhereSymbolListDoesNotHaveLabelDefinition(void) {

    symbols.len = 2;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_LABEL;
    symbols.s[1].value = 0;

    labels.len = 1;
    sprintf(labels.l[0].identifier, "%s", "LABEL");

    int r = resolve_labels(&symbols, &labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0, labels.l[0].byte);
}

void test_resolve_labels_WhereLabelListHasMultipleLabels_WhereSymbolListHasLabelDefinitions(void) {

    symbols.len = 5;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_LABEL_DEFINITION;
    symbols.s[1].value = 0;
    symbols.s[2].type = SYM_INSTRUCTION;
    symbols.s[3].type = SYM_LABEL_DEFINITION;
    symbols.s[3].value = 1;
    symbols.s[4].type = SYM_INSTRUCTION;

    labels.len = 2;
    sprintf(labels.l[0].identifier, "%s", "LABEL");
    sprintf(labels.l[1].identifier, "%s", "OTHERLABEL");

    int r = resolve_labels(&symbols, &labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0x202, labels.l[0].byte);
    TEST_ASSERT_EQUAL_INT(0x204, labels.l[1].byte);
}

void test_resolve_labels_WhereSymbolListIsEmpty(void) {
    labels.len = 1;
    sprintf(labels.l[0].identifier, "%s", "LABEL");

    TEST_ASSERT_EQUAL_INT(0, resolve_labels(&symbols, &labels));
}

void test_resolve_labels_WhereLabelListIsEmpty(void) {

    symbols.len = 1;
    symbols.s[0].type = SYM_DB;

    TEST_ASSERT_EQUAL_INT(1, resolve_labels(&symbols, &labels));
}

void test_substitute_labels_WhereLabelListContainsAllLabels(void) {

    symbols.len = 7;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_LABEL;
    symbols.s[1].value = 1;
    symbols.s[2].type = SYM_LABEL_DEFINITION;
    symbols.s[2].value = 0;
    symbols.s[3].type = SYM_INSTRUCTION;
    symbols.s[4].type = SYM_LABEL_DEFINITION;
    symbols.s[4].value = 1;
    symbols.s[5].type = SYM_INSTRUCTION;
    symbols.s[6].type = SYM_LABEL;
    symbols.s[6].value = 0;

    labels.len = 2;
    labels.l[0].byte = 0x202;
    labels.l[1].byte = 0x204;

    int r = substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(SYM_INT12, symbols.s[1].type);
    TEST_ASSERT_EQUAL_INT(0x204, symbols.s[1].value);
    TEST_ASSERT_EQUAL_INT(SYM_INT12, symbols.s[6].type);
    TEST_ASSERT_EQUAL_INT(0x202, symbols.s[6].value);
}

void test_substitute_labels_WhereLabelListIsMissingLabels(void) {

    symbols.len = 7;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_LABEL;
    symbols.s[1].value = 1;
    symbols.s[2].type = SYM_LABEL_DEFINITION;
    symbols.s[2].value = 0;
    symbols.s[3].type = SYM_INSTRUCTION;
    symbols.s[4].type = SYM_LABEL_DEFINITION;
    symbols.s[4].value = 1;
    symbols.s[5].type = SYM_INSTRUCTION;
    symbols.s[6].type = SYM_LABEL;
    symbols.s[6].value = 0;

    labels.len = 1;
    labels.l[0].byte = 0x202;

    int r = substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(INVALID_SYMBOL_EXCEPTION, r);
}

void test_substitute_labels_WhereSymbolListIsEmpty(void) {
    labels.len = 1;
    labels.l[0].byte = 0x202;

    int r = substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
}

void test_substitute_labels_WhereSymbolListContainsNoLabels_WhereLabelListIsEmpty(void) {

    symbols.len = 4;
    symbols.s[0].type = SYM_INSTRUCTION;
    symbols.s[1].type = SYM_INSTRUCTION;
    symbols.s[2].type = SYM_INSTRUCTION;
    symbols.s[3].type = SYM_DB;

    int r = substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
}

int main(void) {
    symbols.s = calloc(SYMBOL_CEILING, sizeof(symbol_t));
    symbols.ceil = SYMBOL_CEILING;
    labels.l = calloc(LABEL_CEILING, sizeof(label_t));
    labels.ceil = LABEL_CEILING;

    for (fc = 0; formats[fc].cmd != I_NULL; fc++);

    c8_lines = malloc(MAX_LINE_COUNT * sizeof(char*));
    line0 = malloc(MAX_LINE_LEN * MAX_LINE_COUNT);
    for (int i = 0; i < MAX_LINE_COUNT; i++) {
        c8_lines[i] = line0 + (i * MAX_LINE_LEN);
    }

    c8_lines_unformatted = malloc(MAX_LINE_COUNT * sizeof(char*));
    for (int i = 0; i < MAX_LINE_COUNT; i++) {
        c8_lines_unformatted[i] = line0 + (i * MAX_LINE_LEN);
    }

    UNITY_BEGIN();

    RUN_TEST(test_is_comment_WhereCommentIsEntireString);
    RUN_TEST(test_is_comment_WhereCommentIsAtEndOfString);
    RUN_TEST(test_is_comment_WhereNoCommentIsInString);
    RUN_TEST(test_is_comment_WhereStringIsEmpty);

    RUN_TEST(test_is_db_WhereStringIsDB);
    RUN_TEST(test_is_db_WhereStringIsNotDB);
    RUN_TEST(test_is_db_WhereStringContainsDB);
    RUN_TEST(test_is_db_WithTrailingChars);
    RUN_TEST(test_is_db_WhereStringIsEmpty);

    RUN_TEST(test_is_dw_WhereStringIsDW);
    RUN_TEST(test_is_dw_WhereStringIsNotDW);
    RUN_TEST(test_is_dw_WhereStringContainsDW);
    RUN_TEST(test_is_dw_WithTrailingChars);
    RUN_TEST(test_is_dw_WhereStringIsEmpty);

    RUN_TEST(test_is_instruction_WhereStringIsInstruction);
    RUN_TEST(test_is_instruction_WhereStringIsNotInstruction);
    RUN_TEST(test_is_instruction_WhereStringIsEmpty);

    RUN_TEST(test_is_label_definition_WhereStringIsLabelDefinition);
    RUN_TEST(test_is_label_definition_WhereStringIsNotLabelDefinition);
    RUN_TEST(test_is_label_definition_WhereStringIsEmpty);

    RUN_TEST(test_is_label_WhereStringIsLabel);
    RUN_TEST(test_is_label_WhereStringIsNotLabel);
    RUN_TEST(test_is_label_WhereStringIsEmpty);

    RUN_TEST(test_is_register_WhereStringIsRegister_WhereRegisterIsUppercase);
    RUN_TEST(test_is_register_WhereStringIsRegister_RegisterIsLowercase);
    RUN_TEST(test_is_register_WhereStringIsNotRegister);
    RUN_TEST(test_is_register_WhereStringIsEmpty);

    RUN_TEST(test_is_reserved_identifier_WhereStringIsReservedIdentifier);
    RUN_TEST(test_is_reserved_identifier_WhereStringIsNotReservedIdentifier);
    RUN_TEST(test_is_register_WhereStringIsEmpty);

    RUN_TEST(test_next_symbol_WhereSymbolListIsEmpty);
    RUN_TEST(test_next_symbol_WhereSymbolListIsFull);
    RUN_TEST(test_next_symbol_WhereSymbolListIsNotEmptyOrFull);

    RUN_TEST(test_populate_labels_WhereLinesHasDuplicateLabelDefinitions);
    RUN_TEST(test_populate_labels_WhereLinesIsEmpty);
    RUN_TEST(test_populate_labels_WhereLinesHasMultipleLabelDefinitions);
    RUN_TEST(test_populate_labels_WhereLabelListIsEmpty);

    RUN_TEST(test_resolve_labels_WhereLabelListHasOneLabel_WhereSymbolListHasLabelDefinition);
    RUN_TEST(test_resolve_labels_WhereLabelListHasMultipleLabels_WhereSymbolListHasLabelDefinitions);
    RUN_TEST(test_resolve_labels_WhereSymbolListIsEmpty);
    RUN_TEST(test_resolve_labels_WhereLabelListIsEmpty);

    RUN_TEST(test_substitute_labels_WhereLabelListContainsAllLabels);
    RUN_TEST(test_substitute_labels_WhereLabelListIsMissingLabels);
    RUN_TEST(test_substitute_labels_WhereSymbolListIsEmpty);
    RUN_TEST(test_substitute_labels_WhereSymbolListContainsNoLabels_WhereLabelListIsEmpty);

    free(symbols.s);
    free(labels.l);
    free(line0);
    free(c8_lines);
    free(c8_lines_unformatted);
    return UNITY_END();
}
