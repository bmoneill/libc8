#include "unity.h"

#include "c8/encode.h"
#include "c8/private/exception.h"
#include "c8/private/symbol.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_COUNT 10
#define MAX_LINE_LEN   50

C8_Instruction ins;
C8_SymbolList  symbols;
C8_LabelList   labels;
const char*    empty = "\0";
int            fc    = 0;
char*          line0;

void           setUp(void) {
    symbols.s    = calloc(C8_SYMBOL_CEILING, sizeof(C8_Symbol));
    symbols.ceil = C8_SYMBOL_CEILING;
    labels.l     = calloc(C8_LABEL_CEILING, sizeof(C8_Label));
    labels.ceil  = C8_LABEL_CEILING;

    for (fc = 0; c8_formats[fc].cmd != C8_I_NULL; fc++)
        ;

    c8_lines = malloc(MAX_LINE_COUNT * sizeof(char*));
    line0    = malloc(MAX_LINE_LEN * MAX_LINE_COUNT);
    for (int i = 0; i < MAX_LINE_COUNT; i++) {
        c8_lines[i] = line0 + (i * MAX_LINE_LEN);
    }

    c8_linesUnformatted = malloc(MAX_LINE_COUNT * sizeof(char*));
    for (int i = 0; i < MAX_LINE_COUNT; i++) {
        c8_linesUnformatted[i] = line0 + (i * MAX_LINE_LEN);
    }

    memset(&ins, 0, sizeof(C8_Instruction));
    memset(line0, 0, MAX_LINE_LEN * MAX_LINE_COUNT);

    memset(labels.l, 0, C8_LABEL_CEILING * sizeof(C8_Label));
    labels.len  = 0;
    labels.ceil = C8_LABEL_CEILING;

    memset(symbols.s, 0, C8_SYMBOL_CEILING * sizeof(C8_Symbol));
    symbols.len  = 0;
    symbols.ceil = C8_SYMBOL_CEILING;
}

void tearDown(void) {
    free(symbols.s);
    free(labels.l);
    free(line0);
    free(c8_lines);
    free(c8_linesUnformatted);
}

void test_c8_is_comment_WhereCommentIsAtEndOfString(void) {
    const char* s = "Hello ; This is a comment";
    TEST_ASSERT_EQUAL_INT(0, c8_is_comment(s));
}

void test_c8_is_comment_WhereCommentIsEntireString(void) {
    const char* s = "; This is a comment";
    TEST_ASSERT_EQUAL_INT(1, c8_is_comment(s));
}

void test_c8_is_comment_WhereStringIsEmpty(void) { TEST_ASSERT_EQUAL_INT(0, c8_is_comment(empty)); }

void test_c8_is_comment_WhereNoCommentIsInString(void) {
    const char* s = "This is not a comment";
    TEST_ASSERT_EQUAL_INT(0, c8_is_comment(s));
}

void test_c8_is_db_WhereStringIsDB(void) { TEST_ASSERT_EQUAL_INT(1, c8_is_db(C8_S_DB)); }

void test_c8_is_db_WhereStringIsNotDB(void) { TEST_ASSERT_EQUAL_INT(0, c8_is_db(C8_S_DW)); }

void test_c8_is_db_WhereStringContainsDB(void) {
    const char* s = "Foo .DB";
    TEST_ASSERT_EQUAL_INT(0, c8_is_db(s));
}

void test_c8_is_db_WithTrailingChars(void) {
    const char* s = ".DB foo";
    TEST_ASSERT_EQUAL_INT(0, c8_is_db(s));
}

void test_c8_is_db_WhereStringIsEmpty(void) { TEST_ASSERT_EQUAL_INT(0, c8_is_db(empty)); }

void test_c8_is_dw_WhereStringIsDW(void) { TEST_ASSERT_EQUAL_INT(1, c8_is_dw(C8_S_DW)); }

void test_c8_is_dw_WhereStringIsNotDW(void) { TEST_ASSERT_EQUAL_INT(0, c8_is_dw(C8_S_DB)); }

void test_c8_is_dw_WhereStringContainsDW(void) {
    const char* s = "Foo .DW";
    TEST_ASSERT_EQUAL_INT(0, c8_is_dw(s));
}

void test_c8_is_dw_WithTrailingChars(void) {
    const char* s = ".DW foo";
    TEST_ASSERT_EQUAL_INT(0, c8_is_dw(s));
}

void test_c8_is_dw_WhereStringIsEmpty(void) { TEST_ASSERT_EQUAL_INT(0, c8_is_dw(empty)); }

void test_c8_is_instruction_WhereStringIsInstruction(void) {
    int ic = 0;
    for (ic = 0; c8_instructionStrings[ic] != NULL; ic++)
        ;

    int  i = rand() % ic;

    char s[16];
    strcpy(s, c8_instructionStrings[i]);

    TEST_ASSERT_EQUAL_INT(i, c8_is_instruction(s));
}

void test_c8_is_instruction_WhereStringIsNotInstruction(void) {
    const char* s = "Not an instruction";

    TEST_ASSERT_EQUAL_INT(-1, c8_is_instruction(s));
}

void test_c8_is_instruction_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, c8_is_instruction(empty));
}

void test_c8_is_label_definition_WhereStringIsLabelDefinition(void) {
    const char* s = "L:";

    TEST_ASSERT_EQUAL_INT(1, c8_is_label_definition(s));
}

void test_c8_is_label_definition_WhereStringIsNotLabelDefinition(void) {
    const char* s = "L";

    TEST_ASSERT_EQUAL_INT(0, c8_is_label_definition(s));
}

void test_c8_is_label_definition_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(0, c8_is_label_definition(empty));
}

void test_c8_is_label_WhereStringIsLabel(void) {
    const char* s    = "L";

    labels.len       = 3;
    labels.l[0].byte = rand();
    strcpy(labels.l[0].identifier, "LABEL");
    labels.l[1].byte = rand();
    strcpy(labels.l[1].identifier, "ANOTHERLABEL");
    labels.l[2].byte = rand();
    strcpy(labels.l[2].identifier, "L");

    TEST_ASSERT_EQUAL_INT(2, c8_is_label(s, &labels));
}

void test_c8_is_label_WhereStringIsNotLabel(void) {
    const char* s    = "L";

    labels.len       = 3;
    labels.l[0].byte = rand();
    strcpy(labels.l[0].identifier, "LABEL");
    labels.l[1].byte = rand();
    strcpy(labels.l[1].identifier, "ANOTHERLABEL");
    labels.l[2].byte = rand();
    strcpy(labels.l[2].identifier, "L");

    TEST_ASSERT_EQUAL_INT(-1, c8_is_label("LABEL3", &labels));
}

void test_c8_is_label_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, c8_is_label(empty, &labels));
}

void test_c8_is_register_WhereStringIsRegister_WhereRegisterIsUppercase(void) {
    const char* s = "V1";
    TEST_ASSERT_EQUAL_INT(1, c8_is_register(s));
}

void test_c8_is_register_WhereStringIsRegister_RegisterIsLowercase(void) {
    const char* s = "vf";
    TEST_ASSERT_EQUAL_INT(0xF, c8_is_register(s));
}

void test_c8_is_register_WhereStringIsNotRegister(void) {
    const char* s = "x4";
    TEST_ASSERT_EQUAL_INT(-1, c8_is_register(s));
}

void test_c8_is_register_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, c8_is_register(empty));
}

void test_c8_is_reserved_identifier_WhereStringIsReservedIdentifier(void) {
    int ic = 0;
    for (ic = 0; c8_identifierStrings[ic] != NULL; ic++)
        ;

    int  ident = rand() % ic;

    char s[16];
    strcpy(s, c8_identifierStrings[ident]);

    TEST_ASSERT_EQUAL_INT(ident, c8_is_reserved_identifier(s));
}

void test_c8_is_reserved_identifier_WhereStringIsNotReservedIdentifier(void) {
    const char* s = "Not reserved";

    TEST_ASSERT_EQUAL_INT(-1, c8_is_reserved_identifier(s));
}

void test_c8_is_reserved_identifier_WhereStringIsEmpty(void) {
    TEST_ASSERT_EQUAL_INT(-1, c8_is_reserved_identifier(empty));
}

void test_c8_next_symbol_WhereSymbolListIsEmpty(void) {
    memset(symbols.s, 0, C8_SYMBOL_CEILING);
    symbols.len  = 0;
    symbols.ceil = C8_SYMBOL_CEILING;

    TEST_ASSERT_EQUAL_PTR(&symbols.s[0], c8_next_symbol(&symbols));
}

void test_c8_next_symbol_WhereSymbolListIsNotEmptyOrFull(void) {
    memset(symbols.s, 0, C8_SYMBOL_CEILING);

    symbols.len       = 2;
    symbols.ceil      = C8_SYMBOL_CEILING;
    symbols.s[0].type = C8_SYM_INSTRUCTION;
    symbols.s[1].type = C8_SYM_IP;

    TEST_ASSERT_EQUAL_PTR(&symbols.s[2], c8_next_symbol(&symbols));
}

void test_c8_next_symbol_WhereSymbolListIsFull(void) {
    memset(symbols.s, C8_SYM_INSTRUCTION, C8_SYMBOL_CEILING);

    symbols.len       = C8_SYMBOL_CEILING;
    symbols.ceil      = C8_SYMBOL_CEILING;

    C8_Symbol* symbol = c8_next_symbol(&symbols);

    TEST_ASSERT_EQUAL_INT(C8_SYMBOL_CEILING + 1, symbols.len);
    TEST_ASSERT_EQUAL_INT(C8_SYMBOL_CEILING * 2, symbols.ceil);
    TEST_ASSERT_EQUAL_PTR(&symbols.s[C8_SYMBOL_CEILING], symbol);
}

void test_c8_populate_labels_WhereLinesIsEmpty(void) {
    int r = c8_populate_labels(&labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0, labels.len);
    TEST_ASSERT_EQUAL_STRING("\0", labels.l[0].identifier);
}

void test_c8_populate_labels_WhereLabelListIsEmpty(void) {
    sprintf(c8_lines[0], "%s", "ADD V4, V5");
    sprintf(c8_lines[1], "%s", "\0");
    sprintf(c8_lines[2], "%s", "CLS");
    sprintf(c8_lines[3], "%s", "RET");
    sprintf(c8_lines[4], "%s", "SE V1, $55");

    int r = c8_populate_labels(&labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0, labels.len);
    TEST_ASSERT_EQUAL_STRING("\0", labels.l[0].identifier);
}

void test_c8_populate_labels_WhereLinesHasMultipleLabelDefinitions(void) {
    sprintf(c8_lines[0], "%s", "ADD V4, V5");
    sprintf(c8_lines[1], "%s", "label:");
    sprintf(c8_lines[2], "%s", "RET");
    sprintf(c8_lines[3], "%s", "otherlabel:");
    sprintf(c8_lines[4], "%s", "SE V1, $55");
    c8_lineCount = 5;

    int r        = c8_populate_labels(&labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(2, labels.len);
    TEST_ASSERT_EQUAL_STRING("label", labels.l[0].identifier);
    TEST_ASSERT_EQUAL_STRING("otherlabel", labels.l[1].identifier);
}

void test_c8_populate_labels_WhereLinesHasDuplicateLabelDefinitions(void) {
    sprintf(c8_lines[0], "%s", "ADD V4, V5");
    sprintf(c8_lines[1], "%s", "label:");
    sprintf(c8_lines[2], "%s", "RET");
    sprintf(c8_lines[3], "%s", "label:");
    sprintf(c8_lines[4], "%s", "SE V1, $55");
    c8_lineCount = 5;

    int r        = c8_populate_labels(&labels);

    TEST_ASSERT_EQUAL_INT(C8_DUPLICATE_LABEL_EXCEPTION, r);
}

void test_c8_resolve_labels_WhereLabelListHasOneLabel_WhereSymbolListHasLabelDefinition(void) {

    symbols.len        = 3;
    symbols.s[0].type  = C8_SYM_INSTRUCTION;
    symbols.s[1].type  = C8_SYM_LABEL;
    symbols.s[1].value = 0;
    symbols.s[2].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[2].value = 0;
    symbols.s[3].type  = C8_SYM_INSTRUCTION;

    labels.len         = 1;
    sprintf(labels.l[0].identifier, "%s", "LABEL");

    int r = c8_resolve_labels(&symbols, &labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0x202, labels.l[0].byte);
}

void test_c8_resolve_labels_WhereLabelListHasOneLabel_WhereSymbolListDoesNotHaveLabelDefinition(
    void) {

    symbols.len        = 2;
    symbols.s[0].type  = C8_SYM_INSTRUCTION;
    symbols.s[1].type  = C8_SYM_LABEL;
    symbols.s[1].value = 0;

    labels.len         = 1;
    sprintf(labels.l[0].identifier, "%s", "LABEL");

    int r = c8_resolve_labels(&symbols, &labels);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(0, labels.l[0].byte);
}

void test_c8_resolve_labels_WhereLabelListHasMultipleLabelC8_S_WhereSymbolListHasLabelDefinitions(
    void) {

    symbols.len        = 5;
    symbols.s[0].type  = C8_SYM_INSTRUCTION;
    symbols.s[1].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[1].value = 0;
    symbols.s[2].type  = C8_SYM_INSTRUCTION;
    symbols.s[3].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[3].value = 1;
    symbols.s[4].type  = C8_SYM_INSTRUCTION;

    labels.len         = 2;
    sprintf(labels.l[0].identifier, "%s", "LABEL");
    sprintf(labels.l[1].identifier, "%s", "OTHERLABEL");

    int r = c8_resolve_labels(&symbols, &labels);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(0x202, labels.l[0].byte);
    TEST_ASSERT_EQUAL_INT(0x204, labels.l[1].byte);
}

void test_c8_resolve_labels_WhereSymbolListIsEmpty(void) {
    labels.len = 1;
    sprintf(labels.l[0].identifier, "%s", "LABEL");

    TEST_ASSERT_EQUAL_INT(0, c8_resolve_labels(&symbols, &labels));
}

void test_c8_resolve_labels_WhereLabelListIsEmpty(void) {

    symbols.len       = 1;
    symbols.s[0].type = C8_SYM_DB;

    TEST_ASSERT_EQUAL_INT(1, c8_resolve_labels(&symbols, &labels));
}

void test_c8_substitute_labels_WhereLabelListContainsAllLabels(void) {

    symbols.len        = 7;
    symbols.s[0].type  = C8_SYM_INSTRUCTION;
    symbols.s[1].type  = C8_SYM_LABEL;
    symbols.s[1].value = 1;
    symbols.s[2].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[2].value = 0;
    symbols.s[3].type  = C8_SYM_INSTRUCTION;
    symbols.s[4].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[4].value = 1;
    symbols.s[5].type  = C8_SYM_INSTRUCTION;
    symbols.s[6].type  = C8_SYM_LABEL;
    symbols.s[6].value = 0;

    labels.len         = 2;
    labels.l[0].byte   = 0x202;
    labels.l[1].byte   = 0x204;

    int r              = c8_substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_INT(C8_SYM_INT12, symbols.s[1].type);
    TEST_ASSERT_EQUAL_INT(0x204, symbols.s[1].value);
    TEST_ASSERT_EQUAL_INT(C8_SYM_INT12, symbols.s[6].type);
    TEST_ASSERT_EQUAL_INT(0x202, symbols.s[6].value);
}

void test_c8_substitute_labels_WhereLabelListIsMissingLabels(void) {

    symbols.len        = 7;
    symbols.s[0].type  = C8_SYM_INSTRUCTION;
    symbols.s[1].type  = C8_SYM_LABEL;
    symbols.s[1].value = 1;
    symbols.s[2].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[2].value = 0;
    symbols.s[3].type  = C8_SYM_INSTRUCTION;
    symbols.s[4].type  = C8_SYM_LABEL_DEFINITION;
    symbols.s[4].value = 1;
    symbols.s[5].type  = C8_SYM_INSTRUCTION;
    symbols.s[6].type  = C8_SYM_LABEL;
    symbols.s[6].value = 0;

    labels.len         = 1;
    labels.l[0].byte   = 0x202;

    int r              = c8_substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(C8_INVALID_SYMBOL_EXCEPTION, r);
}

void test_c8_substitute_labels_WhereSymbolListIsEmpty(void) {
    labels.len       = 1;
    labels.l[0].byte = 0x202;

    int r            = c8_substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
}

void test_c8_substitute_labels_WhereSymbolListContainsNoLabelC8_S_WhereLabelListIsEmpty(void) {
    symbols.len       = 4;
    symbols.s[0].type = C8_SYM_INSTRUCTION;
    symbols.s[1].type = C8_SYM_INSTRUCTION;
    symbols.s[2].type = C8_SYM_INSTRUCTION;
    symbols.s[3].type = C8_SYM_DB;

    int r             = c8_substitute_labels(&symbols, &labels);

    TEST_ASSERT_EQUAL_INT(1, r);
}
