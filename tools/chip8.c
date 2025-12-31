#include "c8/chip8.h"
#include "c8/font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void usage(const char* argv0);

int         main(int argc, char* argv[]) {
    if (argc < 2) {
        usage(argv[0]);
    }

    C8* c8 = c8_init(NULL, 0);

    if (!c8) {
        usage(argv[0]);
    }

    int   opt;
    char* fontstr = NULL;

    /* Parse args */
    while ((opt = getopt(argc, argv, "c:df:p:P:q:vV")) != -1) {
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
            c8_load_palette_f(c8, optarg);
            break;
        case 'P':
            c8_load_palette_s(c8, optarg);
            break;
        case 'v':
            c8->flags |= C8_FLAG_VERBOSE;
            break;
        case 'q':
            c8_load_quirks(c8, optarg);
            break;
        case 'V':
            printf("%s %s\n", argv[0], c8_version());
            return 0;
        default:
            usage(argv[0]);
        }
    }

    if (fontstr) {
        c8_set_fonts_s(c8, fontstr);
    }

    c8_load_rom(c8, argv[optind]);
    c8_simulate(c8);
    c8_deinit(c8);

    return EXIT_SUCCESS;
}

static void usage(const char* argv0) {
    fprintf(
        stderr,
        "Usage: %s [-dvV] [-c clockspeed] [-f small,big] [-p file] [-P colors] [-q quirks] file\n",
        argv0);
    exit(EXIT_FAILURE);
}
