#include "unity.h"
#include "c8/decode.c"
#include "c8/private/exception.h"
#include "c8/defs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FORMAT_A(a) ((a << 12) & 0xF000)
#define FORMAT_X(x) ((x << 8) & 0x0F00)
#define FORMAT_Y(y) ((y << 4) & 0x00F0)
#define FORMAT_B(b) (b & 0x000F)
#define FORMAT_KK(kk) (kk & 0x00FF)
#define FORMAT_NNN(nnn) (nnn & 0x0FFF)
#define BUILD_INSTRUCTION_AXYB(a, x, y, b) \
	(FORMAT_A(a) | FORMAT_X(x) | FORMAT_Y(y) | FORMAT_B(b))

#define BUILD_INSTRUCTION_AXKK(a, x, kk) \
	(FORMAT_A(a) | FORMAT_X(x) | FORMAT_KK(kk))

#define BUILD_INSTRUCTION_ANNN(a, nnn) \
	(FORMAT_A(a) | FORMAT_NNN(nnn))

#define SHOULD_PARSE(expected, ins) \
    TEST_ASSERT_EQUAL_STRING(expected, c8_decode_instruction(ins, label_map));

uint8_t label_map[C8_MEMSIZE];
char buf[64];

int x = 0;
int y = 0;
int kk = 0;
int b = 0;
int nnn = 0;
const int label = 1;

void setUp(void) {
    srand(time(NULL));
    memset(label_map, 0, 4096);
	x = rand() % 0xF;
	y = rand() % 0xF;
	kk = rand() % 0xFF;
	b = rand() % 0xF;
	nnn = rand() % 0xFFF;
}

void tearDown(void) { }

void test_decode_instruction_WhereInstructionIsCLS(void) {
    SHOULD_PARSE("CLS", 0x00E0);
}

void test_decode_instruction_WhereInstructionIsRET(void) {
    SHOULD_PARSE("RET", 0x00EE);
}

void test_decode_instruction_WhereInstructionIsSCD(void) {
    uint16_t ins = 0x00C0 | b;

    sprintf(buf, "SCD 0x%01X", b);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSCR(void) {
    SHOULD_PARSE("SCR", 0x00FB);
}

void test_decode_instruction_WhereInstructionIsSCL(void) {
    SHOULD_PARSE("SCL", 0x00FC);
}

void test_decode_instruction_WhereInstructionIsEXIT(void) {
    SHOULD_PARSE("EXIT", 0x00FD);
}

void test_decode_instruction_WhereInstructionIsLOW(void) {
    SHOULD_PARSE("LOW", 0x00FE);
}

void test_decode_instruction_WhereInstructionIsHIGH(void) {
    SHOULD_PARSE("HIGH", 0x00FF);
}

void test_decode_instruction_WhereInstructionIsJPNNN(void) {
    int ins = BUILD_INSTRUCTION_ANNN(1, nnn);
    label_map[nnn] = 0;

    sprintf(buf, "JP $%03X", nnn);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsJPNNN_WithLabel(void) {
    int ins = BUILD_INSTRUCTION_ANNN(1, nnn);
    label_map[nnn] = label;

    sprintf(buf, "JP label%d", label);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsCALL(void) {
    int ins = BUILD_INSTRUCTION_ANNN(2, nnn);
    label_map[nnn] = 0;

    sprintf(buf, "CALL $%03X", nnn);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsCALL_WithLabel(void) {
    int ins = BUILD_INSTRUCTION_ANNN(2, nnn);
    label_map[nnn] = label;

    sprintf(buf, "CALL label%d", label);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSEXKK(void) {
    int ins = BUILD_INSTRUCTION_AXKK(3, x, kk);

    sprintf(buf, "SE V%01X, 0x%02X", x, kk);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSNEXKK(void) {
    int ins = BUILD_INSTRUCTION_AXKK(4, x, kk);

    sprintf(buf, "SNE V%01X, 0x%02X", x, kk);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSEXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(5, x, y, 0);

    sprintf(buf, "SE V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDXKK(void) {
    int ins = BUILD_INSTRUCTION_AXKK(6, x, kk);

    sprintf(buf, "LD V%01X, 0x%02X", x, kk);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsADDXKK(void) {
    int ins = BUILD_INSTRUCTION_AXKK(7, x, kk);

    sprintf(buf, "ADD V%01X, 0x%02X", x, kk);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 0);

    sprintf(buf, "LD V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsORXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 1);

    sprintf(buf, "OR V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsANDXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 2);

    sprintf(buf, "AND V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsXORXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 3);

    sprintf(buf, "XOR V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsADDXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 4);

    sprintf(buf, "ADD V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSUBXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 5);

    sprintf(buf, "SUB V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSHRXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 6);

    sprintf(buf, "SHR V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSUBNXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 7);

    sprintf(buf, "SUBN V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSHLXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(8, x, y, 0xE);

    sprintf(buf, "SHL V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSNEXY(void) {
    int ins = BUILD_INSTRUCTION_AXYB(9, x, y, 0);

    sprintf(buf, "SNE V%01X, V%01X", x, y);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDINNN(void) {
    int ins = BUILD_INSTRUCTION_ANNN(0xA, nnn);
    label_map[nnn] = 0;

    sprintf(buf, "LD I, $%03X", nnn);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDINNN_WithLabel(void) {
    int ins = BUILD_INSTRUCTION_ANNN(0xA, nnn);
    label_map[nnn] = label;

    sprintf(buf, "LD I, label%d", label);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsJPV0NNN(void) {
    int ins = BUILD_INSTRUCTION_ANNN(0xB, nnn);
    label_map[nnn] = 0;

    sprintf(buf, "JP V0, $%03X", nnn);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsJPV0NNN_WithLabel(void) {
    int ins = BUILD_INSTRUCTION_ANNN(0xB, nnn);
    label_map[nnn] = label;

    sprintf(buf, "JP V0, label%d", label);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsRNDXKK(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xC, x, kk);

    sprintf(buf, "RND V%01X, 0x%02X", x, kk);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsDRWXYB(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXYB(0xD, x, y, b);

    sprintf(buf, "DRW V%01X, V%01X, 0x%01X", x, y, b);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSKPX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xE, x, 0x9E);

    sprintf(buf, "SKP V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsSKNPX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xE, x, 0xA1);

    sprintf(buf, "SKNP V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDXDT(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x07);

    sprintf(buf, "LD V%01X, DT", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDXK(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x0A);

    sprintf(buf, "LD V%01X, K", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDDTX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x15);

    sprintf(buf, "LD DT, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDSTX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x18);

    sprintf(buf, "LD ST, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsADDIX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x1E);

    sprintf(buf, "ADD I, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDFX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x29);

    sprintf(buf, "LD F, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDHFX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x30);

    sprintf(buf, "LD HF, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDBX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x33);

    sprintf(buf, "LD B, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDIPX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x55);

    sprintf(buf, "LD [I], V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDXIP(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x65);

    sprintf(buf, "LD V%01X, [I]", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDRX(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x75);

    sprintf(buf, "LD R, V%01X", x);
    SHOULD_PARSE(buf, ins);
}

void test_decode_instruction_WhereInstructionIsLDXR(void) {
    uint16_t ins = BUILD_INSTRUCTION_AXKK(0xF, x, 0x85);

    sprintf(buf, "LD V%01X, R", x);
    SHOULD_PARSE(buf, ins);
}
