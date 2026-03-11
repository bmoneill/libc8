/**
 * @file c8/chip8.c
 *
 * Stuff for simulating and modifying `C8`s.
 */

#include "chip8.h"

#include "font.h"

#include "private/debug.h"
#include "private/exception.h"
#include "private/instruction.h"
#include "private/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define C8_DEBUG(c) (c->flags & C8_FLAG_DEBUG)

/**
 * @brief Deinitialize graphics and free c8
 *
 * @param c8 `C8` to deinitialize
 */
void c8_deinit(C8* c8) {
    c8_deinit_graphics();
    free(c8);
}

/**
 * @brief Initialize and return a `C8` with the given flags
 *
 * This function allocates memory for a new `C8` with all values set to 0
 * or their default values, adds the font to memory, inititializes the graphics
 * system, and returns a pointer to the `C8`.
 *
 * @param path path to ROM file
 * @param flags flags
 *
 * @return pointer to initialized `C8`, or NULL on failure.
 */
C8* c8_init(const char* path, int flags) {
    int res;

    C8* c8           = (C8*) calloc(1, sizeof(C8));
    c8->flags        = flags;
    c8->cs           = C8_CLOCK_SPEED;
    c8->colors[1]    = 0xFFFFFF;
    c8->display.mode = C8_DISPLAYMODE_HIGH;
    c8->mode         = C8_MODE_CHIP8;

    if (path != NULL && c8_load_rom(c8, path) != 0) {
        return NULL;
    }

    c8_set_fonts(c8, 0, 0);

    if (!c8_init_graphics()) {
        return NULL;
    }

    return c8;
}

/**
 * @brief Load palette from the given string into `colors`.
 *
 * @param c8 where to store the color codes
 * @param s string to load
 *
 * @return 0 if success, C8_INVALID_PARAMETER_EXCEPTION if failure
 */
int c8_load_palette_s(C8* c8, char* s) {
    char* c[2] = { NULL, NULL };
    int   len  = strlen(s);

    c[0]       = s;
    for (int i = 0; i < len; i++) {
        if (s[i] == ',') {
            s[i] = '\0';
            c[1] = &s[i + 1];
        }
    }

    if (!c[1]) {
        C8_EXCEPTION(C8_INVALID_PARAMETER_EXCEPTION, "Invalid color palette: %s", s);
        return C8_INVALID_PARAMETER_EXCEPTION;
    }

    for (int i = 0; i < 2; i++) {
        if ((c8->colors[i] = c8_parse_int(c[i])) == -1) {
            C8_EXCEPTION(C8_INVALID_PARAMETER_EXCEPTION, "Invalid color palette: %s", s);
            return C8_INVALID_PARAMETER_EXCEPTION;
        }
    }

    return 0;
}

/**
 * @brief Load palette from the given path into `colors`.
 *
 * @param c8 where to store the color codes
 * @param path palette file location
 *
 * @return 0 if success, C8_IO_EXCEPTION or C8_INVALID_PARAMETER_EXCEPTION if failure
 */
int c8_load_palette_f(C8* c8, const char* path) {
    char buf[64];
    buf[0]  = '$';
    FILE* f = fopen(path, "r");
    if (!f) {
        C8_EXCEPTION(C8_IO_EXCEPTION, "Could not open color palette file: %s", path);
        return C8_IO_EXCEPTION;
    }
    for (int i = 0; i < 2; i++) {
        int   c;
        char* s = fgets(buf + 1, 64 - 1, f);
        if (!s) {
            C8_EXCEPTION(C8_INVALID_PARAMETER_EXCEPTION, "Invalid color palette.");
            return C8_INVALID_PARAMETER_EXCEPTION;
        }
        if ((c = c8_parse_int(buf)) == -1 || c > 0xFFFFFF) {
            C8_EXCEPTION(C8_INVALID_PARAMETER_EXCEPTION, "Invalid color palette: %s", buf);
            return C8_INVALID_PARAMETER_EXCEPTION;
        }
        c8->colors[i] = c;
    }

    fclose(f);
    return 0;
}

/**
 * @brief Load quirk flags from string
 *
 * Quirk flags are specified as a string of characters, where each character represents a flag:
 * - `b`: bitwise quirks
 * - `d`: draw quirks
 * - `j`: jump quirks
 * - `l`: load/store quirks
 * - `s`: shift quirks
 *
 * @param c8 where to store flags
 * @param s string to get quirks from
 * @return 0 if success, C8_INVALID_PARAMETER_EXCEPTION if failure
 */
int c8_load_quirks(C8* c8, const char* s) {
    for (size_t i = 0; i < strlen(s); i++) {
        switch (s[i]) {
        case 'b':
            c8->flags ^= C8_FLAG_QUIRK_BITWISE;
            break;
        case 'd':
            c8->flags ^= C8_FLAG_QUIRK_DRAW;
            break;
        case 'j':
            c8->flags ^= C8_FLAG_QUIRK_JUMP;
            break;
        case 'l':
            c8->flags ^= C8_FLAG_QUIRK_LOADSTORE;
            break;
        case 's':
            c8->flags ^= C8_FLAG_QUIRK_SHIFT;
            break;
        default:
            C8_EXCEPTION(C8_INVALID_PARAMETER_EXCEPTION, "Invalid quirk: %c", s[i]);
            return C8_INVALID_PARAMETER_EXCEPTION;
        }
    }
    return 0;
}

/**
 * @brief Load a ROM to `c8->mem` at path `addr`.
 *
 * @param c8 `C8` to store the ROM's contents
 * @param addr path to the ROM
 *
 * @return 0 if success, C8_IO_EXCEPTION or C8_INVALID_PARAMETER_EXCEPTION on error.
 */
int c8_load_rom(C8* c8, const char* addr) {
    FILE*         f;
    unsigned long size;

    f = fopen(addr, "r");
    if (!f) {
        C8_EXCEPTION(C8_IO_EXCEPTION, "Could not open ROM file: %s", addr);
        return C8_IO_EXCEPTION;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if (ftell(f) > (0x1000 - 0x200)) {
        /* File is too big, failure */
        C8_EXCEPTION(C8_INVALID_PARAMETER_EXCEPTION, "ROM file too big: %s", addr);
        return C8_INVALID_PARAMETER_EXCEPTION;
    }
    rewind(f);

    /* Read the file into memory */
    unsigned long l = fread(c8->mem + C8_PROG_START, size, 1, f);
    if (l != 1) {
        C8_EXCEPTION(C8_IO_EXCEPTION, "Error occurred while reading ROM file.");
        return C8_IO_EXCEPTION;
    }
    fclose(f);
    return 0;
}

/**
 * @brief Main interpreter simulation loop. Exits when `c8->running` is 0.
 *
 * @param c8 the `C8` to simulate
 * @return 0 if success, exception code on failure
 */
int c8_simulate(C8* c8) {
    int debugRet;
    int ret;
    int step = 1;

    srand(time(NULL));

    c8->pc      = C8_PROG_START;
    c8->running = 1;

    if ((ret = c8_validate(c8)) != 1) {
        return ret;
    }

    while (c8->running) {
        usleep(1000000 / c8->cs);
        int t = c8_tick(c8->key);

        if (t == -2) {
            /* Quit */
            c8->running = 0;
            continue;
        }

        if (c8->key[16]) {
            /* Enter debug mode */
            c8->flags |= C8_FLAG_DEBUG;
            step = 1;
        }

        if (c8->key[17]) {
            /* Exit debug mode */
            if (C8_DEBUG(c8)) {
                c8->flags ^= C8_FLAG_DEBUG;
            }
        }

        if (C8_DEBUG(c8) && (c8_has_breakpoint(c8, c8->pc) || step)) {
            /* Call debug REPL and process return value */
            debugRet = c8_debug_repl(c8);

            switch (debugRet) {
            case C8_DEBUG_QUIT:
                c8->running = 0;
                continue;
            case C8_DEBUG_STEP:
                step = 1;
                break;
            }
        }

        if (t >= 0 && c8->waitingForKey) {
            /* Waiting for key and a key was pressed */
            c8->V[c8->VK]     = t;
            c8->waitingForKey = 0;
        }

        if (!c8->waitingForKey) {
            /* Not waiting for key, parse next instruction */
            ret = c8_parse_instruction(c8);

            if (ret < 0) {
                return ret;
            }

            c8->pc += ret;

            if (c8->dt > 0) {
                c8->dt--;
            }

            if (c8->st > 0) {
                c8->st--; // TODO sound
            }

            if (c8->st == 0) {
                c8_sound_play();
            }

            if (c8->draw) {
                if (c8_render(&c8->display, c8->colors) < 0) {
                    return C8_GRAPHICS_EXCEPTION;
                }
                c8->draw = 0;
            }
        }
    }
    return 0;
}

/**
 * @brief Validate the state of the chip8 emulator.
 *
 * @param c8 The C8 emulator instance
 * @return int 0 on success, C8_INVALID_STATE_EXCEPTION on failure
 */
int c8_validate(const C8* c8) {
    if (!c8) {
        C8_EXCEPTION(C8_INVALID_STATE_EXCEPTION, "C8 is NULL");
        return C8_INVALID_STATE_EXCEPTION;
    }

    if (c8->pc > C8_MEMSIZE) {
        C8_EXCEPTION(C8_INVALID_STATE_EXCEPTION,
                     "PC out of bounds: 0x%04x > 0x%04x",
                     c8->pc,
                     C8_MEMSIZE)
        return C8_INVALID_STATE_EXCEPTION;
    }

    if (c8->cs <= 0) {
        C8_EXCEPTION(C8_INVALID_STATE_EXCEPTION,
                     "Clock speed cannot be less than or equal to zero: cs=%d",
                     c8->cs)
        return C8_INVALID_STATE_EXCEPTION;
    }

    if (c8->VK < 0 || c8->VK >= 16) {
        C8_EXCEPTION(C8_INVALID_STATE_EXCEPTION, "VK out of bounds (0-15): VK=%d", c8->VK)
        return C8_INVALID_STATE_EXCEPTION;
    }

    if (c8->mode < C8_MODE_CHIP8 || c8->mode > C8_MODE_XOCHIP) {
        C8_EXCEPTION(C8_INVALID_STATE_EXCEPTION, "Invalid mode: mode=%d", c8->mode)
        return C8_INVALID_STATE_EXCEPTION;
    }

    if (c8->display.mode != C8_DISPLAYMODE_LOW && c8->display.mode != C8_DISPLAYMODE_HIGH) {
        C8_EXCEPTION(C8_INVALID_STATE_EXCEPTION, "Invalid display mode: mode=%d", c8->display.mode)
        return C8_INVALID_STATE_EXCEPTION;
    }
    return 0;
}

/**
 * @brief Get the version of libc8.
 *
 * @return const char* version string
 */
const char* c8_version(void) { return C8_VERSION; }
