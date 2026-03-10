#include "c8/decode.h"
#include "c8/encode.h"
#include "c8/private/symbol.h"

#include "unity.h"

#include "util.c"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern C8_InstructionFormat c8_formats[];

void                        setUp(void) { srand(time(NULL)); }
void                        tearDown(void) {}

void                        test_encode_decode(void) {
    C8_InstructionFormat format;
    uint8_t              bytecode[2];
    char asm[32];
    uint16_t input;
    uint16_t output;

    for (int i = 0; c8_formats[i].cmd != C8_I_NULL; i++) {
        // Generate bytecode for each format
        format = c8_formats[i];
        input  = format.base;
        for (int j = 0; j < format.pcount; j++) {
            input |= rand() & format.pmask[j];
        }

        // TODO decode and encode, assert output is the same
        sprintf(asm, "%s", c8_decode_instruction(input, NULL));
        c8_encode(asm, bytecode, 0);
        printf("%s\n", asm);
        output = bytecode[0] << 8 | bytecode[1];
        TEST_ASSERT_EQUAL_UINT16(input, output);
    }
}
