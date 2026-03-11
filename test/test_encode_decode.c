#include "c8/decode.h"
#include "c8/encode.h"
#include "c8/private/symbol.h"

#include "unity.h"

#include "util.c"

#include <stdio.h>

void setUp(void) {}
void tearDown(void) {}

void test_encode_decode(void) {
    uint8_t bytecode[2];
    char asm[32];
    uint16_t output;

    // Decode and encode, assert output is the same
    for (int i = 0; i <= 0xFFFF; i++) {
        bytecode[0] = 0;
        bytecode[1] = 0;
        sprintf(asm, "%s\n", c8_decode_instruction(i, NULL));
        c8_encode(asm, bytecode, 0);
        output = bytecode[0] << 8 | bytecode[1];
        TEST_ASSERT_EQUAL_UINT16(i, output);
    }
}
