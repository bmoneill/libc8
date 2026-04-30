#include "test_debug.h"
#include "util.c"

char       buf[DEBUG_BUFFER_LENGTH];
C8_Command cmd;
C8         c8;

void       setUp(void) {
    memset(buf, 0, DEBUG_BUFFER_LENGTH);
    memset(&cmd, 0, sizeof(C8_Command));
    memset(&c8, 0, sizeof(C8));
    c8.pc        = 0x200;
    c8.tickSpeed = 1;
    memset(stdio_buffer, 0, sizeof(stdio_buffer));
}

void tearDown(void) {}

void test_c8_parse_arg_WhereCommandIsSet_WhereArgIsEmpty(void) {
    cmd.id = C8_CMD_SET;

    REDIRECT_STDOUT;
    int result = c8_parse_arg(&cmd, buf);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Not enough arguments.\n", stdio_buffer);
}

void test_c8_print_help(void) {
    REDIRECT_STDOUT;
    c8_print_help();
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING(C8_DEBUG_HELP_STRING, stdio_buffer);
}

void test_c8_print_quirks(void) {
    REDIRECT_STDOUT;
    c8_print_quirks(C8_FLAG_QUIRK_JUMPING | C8_FLAG_QUIRK_SHIFTING);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("Quirks: js\n", stdio_buffer);
}

void test_c8_print_r_registers(void) {
    int bufIdx = 0;
    for (int i = 0; i < 4; i++) {
        c8.R[i]     = i;
        c8.R[i + 4] = i + 4;
        bufIdx += sprintf(buf + bufIdx, "R%01X: %02X\t\tR%01X: %02X\n", i, i, i + 4, i + 4);
    }
    REDIRECT_STDOUT;
    c8_print_r_registers(&c8);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);
}

void test_c8_print_v_registers(void) {
    int bufIdx = 0;
    for (int i = 0; i < 8; i++) {
        c8.V[i]     = i;
        c8.V[i + 8] = i + 8;
        bufIdx += sprintf(buf + bufIdx, "V%01X: %02X\t\tV%01X: %02X\n", i, i, i + 8, i + 8);
    }
    REDIRECT_STDOUT;
    c8_print_v_registers(&c8);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);
}

void test_c8_print_stack(void) {
    int bufIdx = 0;
    for (int i = 0; i < 8; i++) {
        c8.stack[i]     = i;
        c8.stack[i + 8] = i + 8;
        bufIdx += sprintf(buf + bufIdx,
                          "%01X: $%03X\t\t%01X: $%03X\n",
                          i,
                          c8.stack[i],
                          i + 8,
                          c8.stack[i + 8]);
    }
    REDIRECT_STDOUT;
    c8_print_stack(&c8);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);
}

void test_c8_print_value_WhereValueIsNone(void) {
    cmd.arg.type = C8_ARG_NONE;

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    TEST_ASSERT_GREATER_THAN_INT(0, strlen(stdio_buffer));
}

void test_c8_print_value_WhereValueIsSP(void) {
    cmd.arg.type = C8_ARG_SP;

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING("SP: 00\n", stdio_buffer);
}

void test_c8_print_value_WhereValueIsV(void) {
    cmd.arg.type = C8_ARG_V;

    // Print all V registers
    cmd.arg.value.i = -1;

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    memcpy(buf, stdio_buffer, strlen(stdio_buffer));

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);

    // Print single V register
    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.value.i = 0;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING("V0: 00\n", stdio_buffer);
}

void test_c8_print_value_WhereValueIsR(void) {
    cmd.arg.type    = C8_ARG_R;
    cmd.arg.value.i = -1;

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    memcpy(buf, stdio_buffer, strlen(stdio_buffer));

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);
}

void test_c8_print_value_WhereValueIsInternalRegister(void) {
    // PC
    cmd.arg.type = C8_ARG_PC;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("PC: $200\n", stdio_buffer);

    // DT
    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.type = C8_ARG_DT;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("DT: 00\n", stdio_buffer);

    // ST
    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.type = C8_ARG_ST;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("ST: 00\n", stdio_buffer);

    // I
    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.type = C8_ARG_I;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("I:  000\n", stdio_buffer);

    // VK
    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.type = C8_ARG_VK;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("VK: V0\n", stdio_buffer);
}

void test_c8_print_value_WhereValueIsColor(void) {
    cmd.arg.type = C8_ARG_BG;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("BG: 000000\n", stdio_buffer);

    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.type = C8_ARG_FG;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("FG: 000000\n", stdio_buffer);
}

void test_c8_print_value_WhereValueIsFont(void) {
    cmd.arg.type = C8_ARG_BFONT;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("BFONT: octo\n", stdio_buffer);

    memset(stdio_buffer, 0, sizeof(stdio_buffer));
    cmd.arg.type = C8_ARG_SFONT;
    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_STRING("SFONT: octo\n", stdio_buffer);
}

void test_c8_print_value_WhereValueIsQuirks(void) {
    c8.flags     = C8_FLAG_QUIRK_SHIFTING | C8_FLAG_QUIRK_JUMPING;
    cmd.arg.type = C8_ARG_QUIRKS;

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    memcpy(buf, stdio_buffer, strlen(stdio_buffer));
    buf[strlen(stdio_buffer)] = '\0';

    REDIRECT_STDOUT;
    c8_print_quirks(c8.flags);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);
}

void test_c8_print_value_WhereValueIsStack(void) {
    cmd.arg.type = C8_ARG_STACK;

    REDIRECT_STDOUT;
    c8_print_value(&c8, &cmd);
    RESTORE_STDOUT;

    memcpy(buf, stdio_buffer, strlen(stdio_buffer));
    buf[strlen(stdio_buffer)] = '\0';

    REDIRECT_STDOUT;
    c8_print_stack(&c8);
    RESTORE_STDOUT;

    TEST_ASSERT_EQUAL_STRING(buf, stdio_buffer);
}

void test_c8_print_value_WhereValueIsAddr(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_PRINT;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = 0x200;
    REDIRECT_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    RESTORE_STDOUT;
}

void test_c8_run_command_WhereCommandIsHelp(void) {
    cmd.id = C8_CMD_HELP;

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING(C8_DEBUG_HELP_STRING, stdio_buffer);
}

void test_c8_run_command_WhereCommandIsLoad(void) {
    cmd.id          = C8_CMD_LOAD;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = get_path("state.bin");

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    printf("%s\n", stdio_buffer);
    TEST_ASSERT_EQUAL_INT(0, strlen(stdio_buffer));
}

void test_c8_run_command_WhereCommandIsLoad_WhereFileIsInvalid(void) {
    cmd.id          = C8_CMD_LOAD;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = "foo.bin";

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Failed to load state from foo.bin\n", stdio_buffer);
}

void test_c8_run_command_WhereCommandIsSave(void) {
    cmd.id          = C8_CMD_SAVE;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = "newstate.bin";

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    printf("%s\n", stdio_buffer);
    TEST_ASSERT_EQUAL_INT(0, strlen(stdio_buffer));
}

void test_c8_run_command_WhereCommandIsSave_WhereFileIsInvalid(void) {
    cmd.id          = C8_CMD_SAVE;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = "/foo.bin";

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Invalid file\nFailed to save state to /foo.bin\n", stdio_buffer);
}

void test_c8_run_command_WhereCommandIsLoadFlags(void) {
    cmd.id          = C8_CMD_LOADFLAGS;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = get_path("flags.bin");

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(0, strlen(stdio_buffer));

    cmd.arg.value.s = "empty.txt";
    REDIRECT_STDOUT;
    result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Failed to load flags from empty.txt\n", stdio_buffer);
}

void test_c8_run_command_WhereCommandIsSaveFlags(void) {
    cmd.id          = C8_CMD_SAVEFLAGS;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = "newflags.bin";

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    printf("%s\n", stdio_buffer);
    TEST_ASSERT_EQUAL_INT(0, strlen(stdio_buffer));
}

void test_c8_run_command_WhereCommandIsSaveFlags_WhereFileIsInvalid(void) {
    cmd.id          = C8_CMD_SAVEFLAGS;
    cmd.arg.type    = C8_ARG_FILE;
    cmd.arg.value.s = "/foo.bin";

    REDIRECT_STDOUT;
    int result = c8_run_command(&c8, &cmd);
    RESTORE_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Invalid file\nFailed to save flags to /foo.bin\n", stdio_buffer);
}
