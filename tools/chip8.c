#include "c8/chip8.h"
#include "c8/font.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(const char* argv0);

int         main(int argc, char* argv[]) {
    if (argc < 2) {
        usage(argv[0]);
    }

    C8* c8           = c8_init(NULL, 0);
    c8->display.mode = C8_DISPLAYMODE_LOW;
    c8->mode         = C8_MODE_CHIP8;

    if (!c8) {
        usage(argv[0]);
    }

    int   opt;
    char* fontstr           = NULL;
    int   userDefinedQuirks = 0;

    /* Parse args */
    while ((opt = getopt(argc, argv, "c:df:p:P:q:svV")) != -1) {
        switch (opt) {
        case 'c':
            c8->cs = atoi(optarg);
            break;
        case 'd':
            c8->flags |= C8_FLAG_DEBUG;
            break;
        case 'f':
            fontstr = optarg;
            break;
        case 'p':
            if (c8_load_palette_f(c8, optarg) != 0) {
                return EXIT_FAILURE;
            }
            break;
        case 'P':
            if (c8_load_palette_s(c8, optarg) != 0) {
                return EXIT_FAILURE;
            }
            break;
        case 's':
            c8->mode = C8_MODE_SCHIP;
            break;
        case 'v':
            c8->flags |= C8_FLAG_VERBOSE;
            break;
        case 'q':
            userDefinedQuirks = 1;
            if (c8_load_quirks(c8, optarg) != 0) {
                return EXIT_FAILURE;
            }
            break;
        case 'V':
            printf("%s %s\n", argv[0], c8_version());
            return 0;
        default:
            usage(argv[0]);
        }
    }

    if (!userDefinedQuirks && c8->mode == C8_MODE_CHIP8) {
        c8_load_quirks(c8, "vmc");
    } else if (!userDefinedQuirks && c8->mode == C8_MODE_SCHIP) {
        c8_load_quirks(c8, "csj");
    }

    if (fontstr && c8_set_fonts_s(c8, fontstr) != 0) {
        return EXIT_FAILURE;
    }

    if (c8_load_rom(c8, argv[optind]) != 0) {
        return EXIT_FAILURE;
    }

    c8_simulate(c8);
    c8_deinit(c8);

    return EXIT_SUCCESS;
}

static void usage(const char* argv0) {
    fprintf(
        stderr,
        "Usage: %s [-dsvV] [-c clockspeed] [-f small,big] [-p file] [-P colors] [-q quirks] file\n",
        argv0);
    exit(EXIT_FAILURE);
}
