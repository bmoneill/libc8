#include "c8/private/debug.h"

#include "c8/chip8.h"
#include "c8/font.h"
#include "c8/graphics.h"
#include "c8/private/exception.h"
#include "unity.h"
#include "util.c"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUFLEN 64

#define TEST_COMMAND(s, cmdid, argtype)                                                            \
    strcpy(buf, s);                                                                                \
    TEST_ASSERT_EQUAL_INT(0, c8_get_command(&cmd, buf));                                           \
    TEST_ASSERT_EQUAL_INT(cmdid, cmd.id);                                                          \
    TEST_ASSERT_EQUAL_INT(argtype, cmd.arg.type);

char                 buf[BUFLEN];
C8_Command           cmd;
C8                   c8;

extern int           c8_get_command(C8_Command*, char*);
extern int           c8_load_file_arg(C8_Command*, char*);
extern int           c8_load_flags(C8*, const char*);
extern int           c8_load_state(C8*, const char*);
extern int           c8_parse_arg(C8_Command*, char*);
extern void          c8_print_help(void);
extern void          c8_print_r_registers(const C8*);
extern void          c8_print_stack(const C8*);
extern void          c8_print_v_registers(const C8*);
extern void          c8_print_value(C8*, const C8_Command*);
extern int           c8_run_command(C8*, const C8_Command*);
extern int           c8_save_flags(const C8*, const char*);
extern int           c8_save_state(const C8*, const char*);
extern int           c8_set_value(C8*, const C8_Command*);

extern const uint8_t c8_smallFonts[5][80];
extern const uint8_t c8_bigFonts[3][160];

void                 setUp(void) {
    memset(buf, 0, BUFLEN);
    memset(&cmd, 0, sizeof(C8_Command));
    memset(&c8, 0, sizeof(C8));
    c8.pc = 0x200;
    c8.cs = 1;
    memset(stdout_buffer, 0, sizeof(stdout_buffer));
}
void tearDown(void) {}

void test_c8_debug_repl_WhereInputContainsEOF(void) {
    WRITE_TO_STDIN("\0"); // NULL treated as EOF here
    TEST_ASSERT_EQUAL_INT(C8_DEBUG_QUIT, c8_debug_repl(&c8));
}

void test_c8_debug_repl_WhereInputIsContinueCommand(void) {
    WRITE_TO_STDIN("continue\n\0");
    TEST_ASSERT_EQUAL_INT(C8_DEBUG_CONTINUE, c8_debug_repl(&c8));
}

void test_c8_debug_repl_WhereInputIsNextCommand(void) {
    WRITE_TO_STDIN("next\n\0");
    TEST_ASSERT_EQUAL_INT(C8_DEBUG_STEP, c8_debug_repl(&c8));
}

void test_c8_has_breakpoint_WhereBreakpointExists(void) {
    c8.breakpoints[0x200] = 1;
    TEST_ASSERT_EQUAL_INT(1, c8_has_breakpoint(&c8, 0x200));
}

void test_c8_has_breakpoint_WhereBreakpointDoesNotExist(void) {
    c8.breakpoints[0x200] = 0;
    TEST_ASSERT_EQUAL_INT(0, c8_has_breakpoint(&c8, 0x200));
}

void test_c8_get_command_WhereCommandIsBreak(void) {
    TEST_COMMAND("break", C8_CMD_ADD_BREAKPOINT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsRMBreak(void) {
    TEST_COMMAND("rmbreak", C8_CMD_RM_BREAKPOINT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsContinue(void) {
    TEST_COMMAND("continue", C8_CMD_CONTINUE, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsNext(void) {
    TEST_COMMAND("next", C8_CMD_NEXT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsSet(void) {
    TEST_COMMAND("set PC 0x200", C8_CMD_SET, C8_ARG_PC);
    TEST_ASSERT_EQUAL_INT(0x200, cmd.setValue);
}

void test_c8_get_command_WhereCommandIsSet_WhereArgIsAddr(void) {
    int addr  = 0x20;
    int value = 0x12;

    TEST_COMMAND("set $20 0x12", C8_CMD_SET, C8_ARG_ADDR);
    TEST_ASSERT_EQUAL_INT(addr, cmd.arg.value.i);
    TEST_ASSERT_EQUAL_INT(value, cmd.setValue);
}

void test_c8_get_command_WhereCommandIsLoad(void) {
    TEST_COMMAND("load /path/to/file", C8_CMD_LOAD, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/file", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsSave(void) {
    TEST_COMMAND("save /path/to/file", C8_CMD_SAVE, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/file", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsLoadFlags(void) {
    TEST_COMMAND("loadflags /path/to/flags", C8_CMD_LOADFLAGS, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/flags", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsSaveFlags(void) {
    TEST_COMMAND("saveflags /path/to/flags", C8_CMD_SAVEFLAGS, C8_ARG_FILE);
    TEST_ASSERT_EQUAL_STRING("/path/to/flags", cmd.arg.value.s);
}

void test_c8_get_command_WhereCommandIsPrint_WithArgument(void) {
    TEST_COMMAND("print PC", C8_CMD_PRINT, C8_ARG_PC);
}

void test_c8_get_command_WhereCommandIsPrint_WithNoArguments(void) {
    TEST_COMMAND("print", C8_CMD_PRINT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsHelp(void) {
    TEST_COMMAND("help", C8_CMD_HELP, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsQuit(void) {
    TEST_COMMAND("quit", C8_CMD_QUIT, C8_ARG_NONE);
}

void test_c8_get_command_WhereCommandIsInvalid(void) {
    const char* s = "invalid";
    strcpy(buf, s);
    TEST_ASSERT_NOT_EQUAL_INT(0, c8_get_command(&cmd, buf));
    TEST_ASSERT_EQUAL_INT(C8_CMD_NONE, cmd.id);
    TEST_ASSERT_EQUAL_INT(C8_ARG_NONE, cmd.arg.type);
}

void test_c8_load_file_arg(void) {
    C8_Command cmd;
    char       arg[16] = "my_state.bin";
    TEST_ASSERT_EQUAL_INT(0, c8_load_file_arg(&cmd, arg));
    TEST_ASSERT_EQUAL_STRING("my_state.bin", cmd.arg.value.s);
    TEST_ASSERT_EQUAL_INT(C8_ARG_FILE, cmd.arg.type);
}

void test_c8_load_flags(void) {
    TEST_ASSERT_EQUAL_INT(0, c8_load_flags(&c8, get_path("flags.bin")));
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_INT(i, c8.R[i]);
    }
}

void test_c8_load_state(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_parse_arg_WhereCommandIsLoadSave(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_parse_arg_WhereCommandIsSet_WhereArgIsAddress(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_parse_arg_WhereCommandIsSet_WhereArgIsOther(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_parse_arg_WhereCommandIsSet_WhereArgIsInvalid(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_help(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_quirks(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_r_registers(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_stack(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsNone(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsSP(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsV(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsR(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsInternalRegister(void) {
    // TODO
    // Combine PC, DT, ST, I, VK
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsColor(void) {
    // TODO
    // Combine BG, FG
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsFont(void) {
    // TODO
    // Combine BFONT, SFONT
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsQuirks(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsStack(void) {
    // TODO
    TEST_ASSERT_EQUAL_INT(1, 2);
}

void test_c8_print_value_WhereValueIsAddr(void) {
    // TODO
    C8_Command cmd;
    cmd.id          = C8_CMD_PRINT;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = 0x200;
    REDIRECT_STDOUT;
    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    RESTORE_STDOUT;
    printf("%s", stdout_buffer);
}

void test_c8_run_command_WhereCommandIsAddBreakpoint_WithNoArgument(void) {
    cmd.id       = C8_CMD_ADD_BREAKPOINT;
    cmd.arg.type = C8_ARG_NONE;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(1, c8.breakpoints[0x200]);
}

void test_c8_run_command_WhereCommandIsAddBreakpoint_WithArgument(void) {
    int addr        = 0x246;
    cmd.id          = C8_CMD_ADD_BREAKPOINT;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = addr;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(1, c8.breakpoints[addr]);
}

void test_c8_run_command_WhereCommandIsRMBreakpoint_WithNoArgument(void) {
    c8.breakpoints[c8.pc] = 1;
    cmd.id                = C8_CMD_RM_BREAKPOINT;
    cmd.arg.type          = C8_ARG_NONE;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0, c8.breakpoints[c8.pc]);
}

void test_c8_run_command_WhereCommandIsRMBreakpoint_WithArgument(void) {
    int addr             = 0x123;
    c8.breakpoints[addr] = 1;
    cmd.id               = C8_CMD_RM_BREAKPOINT;
    cmd.arg.type         = C8_ARG_ADDR;
    cmd.arg.value.i      = addr;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0, c8.breakpoints[addr]);
}

void test_c8_run_command_WhereCommandIsContinue(void) {
    cmd.id = C8_CMD_CONTINUE;

    TEST_ASSERT_EQUAL_INT(C8_DEBUG_CONTINUE, c8_run_command(&c8, &cmd));
}

void test_c8_run_command_WhereCommandIsNext(void) {
    cmd.id = C8_CMD_NEXT;

    TEST_ASSERT_EQUAL_INT(C8_DEBUG_STEP, c8_run_command(&c8, &cmd));
}

void test_c8_run_command_WhereCommandIsQuit(void) {
    cmd.id = C8_CMD_QUIT;

    TEST_ASSERT_EQUAL_INT(C8_DEBUG_QUIT, c8_run_command(&c8, &cmd));
}

void test_c8_run_command_WhereCommandIsSet_WhereArgIsPC(void) {
    cmd.id       = C8_CMD_SET;
    cmd.arg.type = C8_ARG_PC;
    cmd.setValue = 0x300;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(0x300, c8.pc);
}

void test_c8_run_command_WhereCommandIsSet_WhereArgIsADDR(void) {
    int addr        = 0x10;
    int value       = 0x23;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = addr;
    cmd.setValue    = value;

    TEST_ASSERT_EQUAL_INT(0, c8_run_command(&c8, &cmd));
    TEST_ASSERT_EQUAL_INT(value, c8.mem[addr]);
}

void test_c8_save_flags_WhereOutputFileIsValid(void) {
    for (int i = 0; i < 8; i++) {
        c8.R[i] = i;
    }

    int result = c8_save_flags(&c8, "my_flags.bin");
    TEST_ASSERT_EQUAL_INT(0, result);

    // Verify the flags were saved correctly
    uint8_t flags[8];
    FILE*   fp = fopen("my_flags.bin", "rb");
    TEST_ASSERT_NOT_NULL(fp);
    fread(flags, sizeof(uint8_t), 8, fp);
    fclose(fp);
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_INT(c8.R[i], flags[i]);
    }
}

void test_c8_save_flags_WhereOutputFileIsInvalid(void) {
    int result = c8_save_flags(&c8, "");
    TEST_ASSERT_EQUAL_INT(C8_IO_EXCEPTION, result);

    result = c8_save_flags(&c8, NULL);
    TEST_ASSERT_EQUAL_INT(C8_IO_EXCEPTION, result);
}

void test_c8_save_state_WhereOutputFileIsValid(void) {
    int result = c8_save_state(&c8, "my_state.bin");
    TEST_ASSERT_EQUAL_INT(0, result);

    // Verify the state was saved correctly
    // draw is set to 1 in c8_load_state to force display update,
    // so we don't need to check it here
    C8 loaded_c8;
    result = c8_load_state(&loaded_c8, "my_state.bin");
    TEST_ASSERT_EQUAL_INT(0, result);

    for (int i = 0; i < C8_MEMSIZE; i++) {
        TEST_ASSERT_EQUAL_INT(c8.mem[i], loaded_c8.mem[i]);
        TEST_ASSERT_EQUAL_INT(c8.breakpoints[i], loaded_c8.breakpoints[i]);
    }

    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL_INT(c8.V[i], loaded_c8.V[i]);
    }

    for (int i = 0; i < 18; i++) {
        TEST_ASSERT_EQUAL_INT(c8.key[i], loaded_c8.key[i]);
    }

    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_INT(c8.R[i], loaded_c8.R[i]);
    }

    TEST_ASSERT_EQUAL_INT(c8.colors[0], loaded_c8.colors[0]);
    TEST_ASSERT_EQUAL_INT(c8.colors[1], loaded_c8.colors[1]);
    TEST_ASSERT_EQUAL_INT(c8.cs, loaded_c8.cs);

    TEST_ASSERT_EQUAL_INT(c8.display.mode, loaded_c8.display.mode);
    TEST_ASSERT_EQUAL_INT(c8.display.x, loaded_c8.display.x);
    TEST_ASSERT_EQUAL_INT(c8.display.y, loaded_c8.display.y);
    for (int i = 0; i < C8_HIGH_DISPLAY_WIDTH * C8_HIGH_DISPLAY_HEIGHT; i++) {
        TEST_ASSERT_EQUAL_INT(c8.display.p[i], loaded_c8.display.p[i]);
    }

    TEST_ASSERT_EQUAL_INT(c8.flags, loaded_c8.flags);
    TEST_ASSERT_EQUAL_INT(c8.waitingForKey, loaded_c8.waitingForKey);
    TEST_ASSERT_EQUAL_INT(c8.running, loaded_c8.running);
    TEST_ASSERT_EQUAL_INT(c8.pc, loaded_c8.pc);
    TEST_ASSERT_EQUAL_INT(c8.I, loaded_c8.I);
    TEST_ASSERT_EQUAL_INT(c8.sp, loaded_c8.sp);
    TEST_ASSERT_EQUAL_INT(c8.dt, loaded_c8.dt);
    TEST_ASSERT_EQUAL_INT(c8.st, loaded_c8.st);
    TEST_ASSERT_EQUAL_INT(c8.VK, loaded_c8.VK);
}

void test_c8_save_state_WhereOutputFileIsInvalid(void) {
    int result = c8_save_state(&c8, "");
    TEST_ASSERT_EQUAL_INT(C8_IO_EXCEPTION, result);

    result = c8_save_state(&c8, NULL);
    TEST_ASSERT_EQUAL_INT(C8_IO_EXCEPTION, result);
}

void test_c8_set_value_WhereValueIsAddr(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_ADDR;
    cmd.arg.value.i = 0x200;
    cmd.setValue    = 123;

    c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(123, c8.mem[0x200]);
}

void test_c8_set_value_WhereValueIsInternalRegister(void) {
    // TODO
    C8_Command cmd;
    cmd.id       = C8_CMD_SET;
    cmd.setValue = 123;

    cmd.arg.type = C8_ARG_PC;
    int result   = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.pc);

    cmd.arg.type = C8_ARG_DT;
    result       = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.dt);

    cmd.arg.type = C8_ARG_ST;
    result       = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.st);

    cmd.arg.type = C8_ARG_I;
    result       = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.I);

    cmd.arg.type = C8_ARG_VK;
    cmd.setValue = 1;
    result       = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, c8.VK);
}

void test_c8_set_value_WhereValueIsVRegister(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_V;
    cmd.arg.value.i = 1;
    cmd.setValue    = 123;

    int result      = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.V[1]);
}

void test_c8_set_value_WhereValueIsRRegister(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_R;
    cmd.arg.value.i = 1;
    cmd.setValue    = 123;

    int result      = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.R[1]);
}

void test_c8_set_value_WhereValueIsColor(void) {
    C8_Command cmd;
    cmd.id       = C8_CMD_SET;
    cmd.arg.type = C8_ARG_BG;
    cmd.setValue = 123;

    int result   = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, c8.colors[0]);

    cmd.arg.type = C8_ARG_FG;
    cmd.setValue = 456;

    result       = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(456, c8.colors[1]);
}

void test_c8_set_value_WhereValueIsQuirks(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_QUIRKS;
    cmd.arg.value.s = "bdj";

    int result      = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(C8_FLAG_QUIRK_BITWISE | C8_FLAG_QUIRK_DRAW | C8_FLAG_QUIRK_JUMP,
                          c8.flags);

    cmd.arg.value.s = "x";

    result          = c8_set_value(&c8, &cmd);
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_c8_set_value_WhereValueIsBFont(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_BFONT;
    cmd.arg.value.s = "fish";

    int result      = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);

    for (int i = 0; i < 160; i++) {
        TEST_ASSERT_EQUAL_INT(c8_bigFonts[C8_BIGFONT_FISH][i], c8.mem[C8_HIGH_FONT_START + i]);
    }
}

void test_c8_set_value_WhereValueIsSFont(void) {
    C8_Command cmd;
    cmd.id          = C8_CMD_SET;
    cmd.arg.type    = C8_ARG_SFONT;
    cmd.arg.value.s = "fish";

    int result      = c8_set_value(&c8, &cmd);
    TEST_ASSERT_EQUAL_INT(0, result);

    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_INT(c8_smallFonts[C8_SMALLFONT_FISH][i], c8.mem[C8_FONT_START + i]);
    }
}

void test_c8_set_value_WhereValueIsInvalid(void) {
    // TODO
    C8_Command cmd;
    cmd.id = C8_CMD_NONE;
    TEST_ASSERT_NOT_EQUAL_INT(0, c8_set_value(&c8, &cmd));
}
