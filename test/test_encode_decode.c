#include "c8/decode.h"
#include "c8/encode.h"
#include "c8/private/symbol.h"

#include "unity.h"

#include "util.c"

extern C8_InstructionFormat c8_formats[];

void                        setUp(void) {}
void                        tearDown(void) {}

void                        test_encode_decode(void) {
    C8_InstructionFormat format;
    uint8_t              gen_bytecode[2];
    char                 gen_asm[16];
    uint8_t              input[2];
    uint16_t             params[3];

    for (int i = 0; c8_formats[i].cmd != C8_I_NULL; i++) {
        // Generate bytecode for each format
        format   = c8_formats[i];
        input[0] = (format.base >> 8) & 0xFF;
        input[1] = format.base & 0xFF;
        for (int j = 0; j < format.pcount; j++) {
            // TODO Finish
        }

        // TODO decode and encode, assert output is the same
    }
}
