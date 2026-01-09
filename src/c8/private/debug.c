/**
 * @file c8/private/debug.c
 * @note NOT EXPORTED
 *
 * Stuff related to debug mode.
 */

#include "debug.h"

#include "../chip8.h"
#include "../decode.h"
#include "../font.h"
#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

C8_STATIC int  c8_get_command(C8_Command*, char*);
C8_STATIC int  c8_load_file_arg(C8_Command*, char*);
C8_STATIC void c8_load_flags(C8*, const char*);
C8_STATIC void c8_load_state(C8*, const char*);
C8_STATIC int  c8_parse_arg(C8_Command*, char*);
C8_STATIC void c8_print_help(void);
C8_STATIC void c8_print_r_registers(const C8*);
C8_STATIC void c8_print_stack(const C8*);
C8_STATIC void c8_print_v_registers(const C8*);
C8_STATIC void c8_print_value(C8*, const C8_Command*);
C8_STATIC int  c8_run_command(C8*, const C8_Command*);
C8_STATIC void c8_save_flags(const C8*, const char*);
C8_STATIC void c8_save_state(const C8*, const char*);
C8_STATIC int  c8_set_value(C8*, const C8_Command*);

/**
 * These are string values of all possible argument, ordered to match the
 * C8_Argument enumerator.
 */
const char* c8_args[] = {
    "SP", "DT", "ST", "PC", "I", "VK", "stack", "bg", "fg", "sfont", "bfont", "quirks",
};

/**
 * These are string values of all possible commands, ordered to match the
 * C8_Command enumerator.
 */
const char* c8_cmds[] = {
    "break", "rmbreak", "continue", "next", "set",       "load",
    "save",  "print",   "help",     "quit", "loadflags", "saveflags",
};

/**
 * @brief Debug command line loop.
 *
 * This function parses user commands from stdin and prints the result until
 * one of the following conditions is met:
 *
 * - continue command is evaluated (return `C8_DEBUG_CONTINUE`)
 *
 * - quit command is evaluated (return `C8_DEBUG_QUIT`)
 *
 * - next command is evaluated (return `C8_DEBUG_STEP`)
 *
 * @param c8 the current CHIP-8 state
 * @return `C8_DEBUG_CONTINUE`, `C8_DEBUG_STEP`, or `C8_DEBUG_QUIT`
 */
int c8_debug_repl(C8* c8) {
    char       buf[64];
    C8_Command cmd;

    printf("debug > ");
    while (scanf("%63[^\n]", buf) != EOF) {
        if (c8_get_command(&cmd, buf)) {
            int result = c8_run_command(c8, &cmd);
            if (result != 0) {
                return result;
            }
        } else {
            printf("Invalid command\n");
        }
        printf("debug > ");

        // Consume newline
        if (getchar() == EOF) {
            break;
        }
    }

    return C8_DEBUG_QUIT; // EOF
}

/**
 * @brief Check if breakpoint exists at address pc
 *
 * This function checks if there is a breakpoint set at the
 * specified program counter (pc) address in the `C8` structure.
 * It returns 1 if a breakpoint exists at that address,
 * and 0 if no breakpoint is set.
 *
 * @param c8 `C8` to check breakpoints of
 * @param pc address to check for breakpoint at
 * @return 1 if yes, 0 if no
 */
int c8_has_breakpoint(C8* c8, uint16_t pc) { return c8->breakpoints[pc]; }

/**
 * @brief Parse command from string `s` and store in `cmd`.
 *
 * This function attempts to match the command string `s` against
 * a predefined list of commands. If a match is found, it populates
 * the `cmd` structure with the command ID and any associated arguments.
 *
 * @param cmd where to store the command attributes
 * @param s command string
 * @return 1 if successful, 0 if not
 */
C8_STATIC int c8_get_command(C8_Command* cmd, char* s) {
    int numCmds = (int) sizeof(c8_cmds) / sizeof(c8_cmds[0]);

    /* reset cmd */
    cmd->id          = C8_CMD_NONE;
    cmd->arg.value.i = -1;
    cmd->arg.type    = C8_ARG_NONE;
    cmd->setValue    = -1;

    s                = c8_trim(s);
    for (int i = 0; i < numCmds; i++) {
        const char* full = c8_cmds[i];
        size_t      len  = strlen(full);

        if (!strncmp(s, full, len)) {
            /* Full cmd */
            cmd->id = (C8_CommandIdentifier) i;
            if (s[len] == '\0') {
                /* No arg */
                cmd->arg.type = C8_ARG_NONE;
                return 1;
            } else if (isspace(s[len])) {
                /* With arg */
                return c8_parse_arg(cmd, c8_trim(s + len));
            }
        }
    }

    return 0; // Unknown command
}

/**
 * @brief Load flag registers from file.
 *
 * @param c8 struct to load to
 * @param path path to load from
 */
C8_STATIC void c8_load_flags(C8* c8, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Invalid file\n");
        return;
    }

    int ret = fread(&c8->R, 1, 8, f);
    fclose(f);
}

/**
 * @brief Load `C8` from file.
 *
 * @param c8 struct to load to
 * @param path path to load from
 */
C8_STATIC void c8_load_state(C8* c8, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Invalid file\n");
        return;
    }
    int ret = fread(c8, sizeof(C8), 1, f);
    fclose(f);

    c8->draw = 1;
}

/**
 * Load a file path string into cmd. This does not check
 * whether the file exists or can be read from.
 *
 * @param cmd where to store the path
 * @param arg the argument to store
 *
 * @return 1
 */
C8_STATIC int c8_load_file_arg(C8_Command* cmd, char* arg) {
    cmd->arg.type    = C8_ARG_FILE;
    cmd->arg.value.s = c8_trim(arg);
    return 1;
}

/**
 * @brief Parse arguments.
 *
 * @param cmd where to store the argument (cmd->id must be correct)
 * @param s arg string (user input after command)
 *
 * @return 1 if success, 0 otherwise
 */
C8_STATIC int c8_parse_arg(C8_Command* cmd, char* s) {
    C8_Arg* arg       = &cmd->arg;
    char*   value     = NULL;
    int     argsCount = sizeof(c8_args) / sizeof(c8_args[0]);

    arg->type         = C8_ARG_NONE;

    if (cmd->id == C8_CMD_SET) {
        /* Split attribute to set and value to set it to */
        for (size_t i = 0; i < strlen(s); i++) {
            if (isspace(s[i])) {
                s[i]  = '\0';
                value = c8_trim(&s[i + 1]);
            }
        }
    }

    /* Try to match with keywords */
    for (int i = 0; i < argsCount; i++) {
        if (!strcmp(s, c8_args[i])) {
            printf("%s", s);
            arg->type = (C8_ArgIdentifier) i;
        }
    }

    if (cmd->id == C8_CMD_SET) {
        if (!value || strlen(value) == 0) {
            printf("Not enough arguments.\n");
            return 0;
        }
        switch (arg->type) {
        case C8_ARG_ADDR:
            cmd->arg.value.i = c8_parse_int(s);
            cmd->setValue    = c8_parse_int(value);
            return 1;
        case C8_ARG_QUIRKS:
        case C8_ARG_SFONT:
        case C8_ARG_BFONT:
            cmd->arg.value.s = value;
            break;
        default:
            cmd->setValue = c8_parse_int(value);
            break;
        }
    }

    for (int i = 0; i < argsCount; i++) {
        if (!strcmp(s, c8_args[i])) {
            arg->type = (C8_ArgIdentifier) i;
        }
    }

    switch (cmd->id) {
    case C8_CMD_LOAD:
    case C8_CMD_SAVE:
    case C8_CMD_LOADFLAGS:
    case C8_CMD_SAVEFLAGS:
        return c8_load_file_arg(cmd, s);
    default:
        break;
    }

    switch (s[0]) {
    case 'V':
    case 'R':
        arg->type = s[0] == 'V' ? C8_ARG_V : C8_ARG_R;
        if (strlen(s) > 1) {
            arg->value.i = c8_hex_to_int(s[1]);
        } else {
            arg->value.i = C8_ARG_NONE;
        }
        break;
    case '$':
        arg->type    = C8_ARG_ADDR;
        arg->value.i = c8_parse_int(s);
        return arg->value.i > 0;
    default:
        break;
    }

    return 1;
}

/**
 * @brief Print the help string.
 */
C8_STATIC void c8_print_help(void) { printf("%s\n", C8_DEBUG_HELP_STRING); }

/**
 * @brief print quirk identifiers in `flags`
 *
 * @param flags flags to get enabled quirks from
 */
C8_STATIC void print_quirks(int flags) {
    int f = 0;
    printf("Quirks: ");
    if (flags & C8_FLAG_QUIRK_BITWISE) {
        f = 1;
        printf("b");
    }
    if (flags & C8_FLAG_QUIRK_DRAW) {
        f = 1;
        printf("d");
    }
    if (flags & C8_FLAG_QUIRK_JUMP) {
        f = 1;
        printf("j");
    }
    if (flags & C8_FLAG_QUIRK_LOADSTORE) {
        f = 1;
        printf("l");
    }
    if (flags & C8_FLAG_QUIRK_SHIFT) {
        f = 1;
        printf("s");
    }
    if (!f) {
        printf("None");
    }
    printf("\n");
}

/**
 * @brief Print all R (flag) registers.
 *
 * @param c8 the current CHIP-8 state
 */
C8_STATIC void c8_print_r_registers(const C8* c8) {
    for (int i = 0; i < 4; i++) {
        printf("R%01x: %02x\t\t", i, c8->R[i]);
        printf("R%01x: %02x\n", i + 4, c8->R[i + 4]);
    }
}

/**
 * @brief Print all V registers (V0-Vf).
 *
 * @param c8 the current CHIP-8 state
 */
C8_STATIC void c8_print_v_registers(const C8* c8) {
    for (int i = 0; i < 8; i++) {
        printf("V%01x: %02x\t\t", i, c8->V[i]);
        printf("V%01x: %02x\n", i + 8, c8->V[i + 8]);
    }
}

/**
 * @brief Print all elements of the stack.
 *
 * @param c8 the current CHIP-8 state
 */
C8_STATIC void c8_print_stack(const C8* c8) {
    for (int i = 0; i < 8; i++) {
        printf("x%01x: $%03x\t\t", i, c8->stack[i]);
        printf("x%01x: $%03x\n", i + 8, c8->stack[i + 8]);
    }
}

/**
 * @brief Print the value specified by the arg in cmd.
 *
 * @param c8 the current CHIP-8 state
 * @param cmd the command structure to get the arg from
 */
C8_STATIC void c8_print_value(C8* c8, const C8_Command* cmd) {
    uint16_t pc;
    uint16_t ins;
    int      addr;

    switch (cmd->arg.type) {
    case C8_ARG_NONE:
        pc  = c8->pc;
        ins = (((uint16_t) c8->mem[pc]) << 8) | c8->mem[pc + 1];

        printf("$%03x: %04x\t%s\n", pc, ins, c8_decode_instruction(ins, NULL));
        printf("PC: %03x\t\tSP: %02x\n", c8->pc, c8->sp);
        printf("DT: %02x\t\tST: %02x\n", c8->dt, c8->st);
        printf("I:  %03x\t\tK:  V%01x\n", c8->I, c8->VK);
        printf("BG: %06x\tFG: %06x\n", c8->colors[0], c8->colors[1]);
        c8_print_fonts(c8);
        c8_print_v_registers(c8);
        c8_print_r_registers(c8);
        printf("Stack:\n");
        c8_print_stack(c8);
        print_quirks(c8->flags);
        break;
    case C8_ARG_SP:
        printf("SP: %02x\n", c8->sp);
        break;
    case C8_ARG_V:
        if (cmd->arg.value.i == -1) {
            c8_print_v_registers(c8);
        } else {
            printf("V%01x: %02x\n", cmd->arg.value.i, c8->V[cmd->arg.value.i]);
        }
        break;
    case C8_ARG_R:
        if (cmd->arg.value.i == -1) {
            c8_print_r_registers(c8);
        } else {
            printf("R%01x: %02x\n", cmd->arg.value.i, c8->R[cmd->arg.value.i]);
        }
        break;
    case C8_ARG_PC:
        printf("PC: %03x\n", c8->pc);
        break;
    case C8_ARG_DT:
        printf("DT: %02x\n", c8->dt);
        break;
    case C8_ARG_ST:
        printf("ST: %02x\n", c8->st);
        break;
    case C8_ARG_I:
        printf("I:  %03x\n", c8->I);
        break;
    case C8_ARG_VK:
        printf("VK: V%01x\n", c8->VK);
        break;
    case C8_ARG_BG:
        printf("BG: %06x\n", c8->colors[0]);
        break;
    case C8_ARG_FG:
        printf("FG: %06x\n", c8->colors[1]);
        break;
    case C8_ARG_BFONT:
        printf("BFONT: %s\n", c8_fontNames[1][c8->fonts[1]]);
        break;
    case C8_ARG_SFONT:
        printf("SFONT: %s\n", c8_fontNames[0][c8->fonts[0]]);
        break;
    case C8_ARG_QUIRKS:
        print_quirks(c8->flags);
        break;
    case C8_ARG_STACK:
        c8_print_stack(c8);
        break;
    case C8_ARG_ADDR:
        addr = cmd->arg.value.i;
        printf("$%03x: %04x\t%s\n",
               addr,
               c8->mem[addr],
               c8_decode_instruction(c8->mem[addr], NULL));
        break;
    default:
        break; // Should not be reached
    }
}

/**
 * @brief Run the command specified in `cmd`.
 *
 * This function executes the command specified in the `cmd` structure.
 *
 * @param c8 the current CHIP-8 state
 * @param cmd the command structure containing the command ID and arguments
 * @return `C8_DEBUG_CONTINUE`, `C8_DEBUG_STEP`, `C8_DEBUG_QUIT`, or 0
 */
C8_STATIC int c8_run_command(C8* c8, const C8_Command* cmd) {
    switch (cmd->id) {
    case C8_CMD_ADD_BREAKPOINT:
        if (cmd->arg.type == C8_ARG_NONE) {
            c8->breakpoints[c8->pc] = 1;
        } else {
            c8->breakpoints[cmd->arg.value.i] = 1;
        }
        break;
    case C8_CMD_RM_BREAKPOINT:
        if (cmd->arg.type == C8_ARG_NONE) {
            c8->breakpoints[c8->pc] = 0;
        } else {
            c8->breakpoints[cmd->arg.value.i] = 0;
        }
        break;
    case C8_CMD_CONTINUE:
        return C8_DEBUG_CONTINUE;
    case C8_CMD_NEXT:
        return C8_DEBUG_STEP;
    case C8_CMD_LOAD:
        c8_load_state(c8, cmd->arg.value.s);
        break;
    case C8_CMD_SAVE:
        c8_save_state(c8, cmd->arg.value.s);
        break;
    case C8_CMD_PRINT:
        c8_print_value(c8, cmd);
        break;
    case C8_CMD_SET:
        c8_set_value(c8, cmd);
        break;
    case C8_CMD_HELP:
        c8_print_help();
        break;
    case C8_CMD_QUIT:
        return C8_DEBUG_QUIT;
    case C8_CMD_LOADFLAGS:
        c8_load_flags(c8, cmd->arg.value.s);
        break;
    case C8_CMD_SAVEFLAGS:
        c8_save_flags(c8, cmd->arg.value.s);
        break;
    default:
        printf("Invalid command\n");
        break;
    }

    return 0;
}

/**
 * @brief Save flag registers to file.
 *
 * @param c8 `C8` to grab flag registers from
 * @param path path to save to
 */
C8_STATIC void c8_save_flags(const C8* c8, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        printf("Invalid file\n");
        return;
    }

    fwrite(&c8->R, 1, 8, f);
    fclose(f);
}

/**
 * @brief Save `C8` to file.
 *
 * @param c8 `C8` to save
 * @param path path to save to
 */
C8_STATIC void c8_save_state(const C8* c8, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        printf("Invalid file\n");
        return;
    }

    fwrite(c8, sizeof(C8), 1, f);
    fclose(f);
}

/**
 * @brief Set the value at `cmd->arg.type` to `cmd->setValue`
 *
 * This assumes the arguments are correctly formatted (e.g. no `ARG_V` type
 * without a value).
 *
 * @param c8 the current CHIP-8 state
 * @param cmd the command structure
 */
C8_STATIC int c8_set_value(C8* c8, const C8_Command* cmd) {
    switch (cmd->arg.type) {
    case C8_ARG_NONE:
        return 0;
    case C8_ARG_ADDR:
        c8->mem[cmd->arg.value.i] = cmd->setValue;
        return 1;
    case C8_ARG_DT:
        c8->dt = cmd->setValue;
        return 1;
    case C8_ARG_I:
        c8->I = cmd->setValue;
        return 1;
    case C8_ARG_PC:
        c8->pc = cmd->setValue;
        return 1;
    case C8_ARG_SP:
        c8->sp = cmd->setValue;
        return 1;
    case C8_ARG_ST:
        c8->st = cmd->setValue;
        return 1;
    case C8_ARG_V:
        c8->V[cmd->arg.value.i] = cmd->setValue;
        return 1;
    case C8_ARG_VK:
        c8->VK = cmd->arg.value.i;
        return 1;
    case C8_ARG_BG:
        c8->colors[0] = cmd->arg.value.i;
        return 1;
    case C8_ARG_FG:
        c8->colors[1] = cmd->arg.value.i;
        return 1;
    case C8_ARG_QUIRKS:
        c8_load_quirks(c8, cmd->arg.value.s);
        return 1;
    case C8_ARG_BFONT:
        c8_set_big_font(c8, cmd->arg.value.s);
        return 1;
    case C8_ARG_SFONT:
        c8_set_small_font(c8, cmd->arg.value.s);
        return 1;
    default:
        printf("Invalid argument\n");
        return 0;
    }
}
