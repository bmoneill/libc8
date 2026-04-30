#include <stdio.h>
#include <unistd.h>

// These macros redirect stdout and stderr, only guaranteed to work
// on Linux. For some reason, macOS doesn't like this.
// All tests using these macros are skipped on macOS, but success on
// Linux implies the same behavior on macOS, so it's not a huge deal.
#define REDIRECT_STDOUT                                                                            \
    freopen("/dev/null", "a", stdout);                                                             \
    setbuf(stdout, stdio_buffer);
#define RESTORE_STDOUT freopen("/dev/tty", "w", stdout);
#define REDIRECT_STDERR                                                                            \
    freopen("/dev/null", "a", stderr);                                                             \
    setbuf(stderr, stdio_buffer);
#define RESTORE_STDERR freopen("/dev/tty", "w", stderr);

#define TEST_DATA_DIR_1 "test/data/"
#define TEST_DATA_DIR_2 "data/"
#define TEST_DATA_DIR_3 "../test/data/"
#define TEST_DATA_DIR_4 "../../test/data/"
#define BUFFER_LENGTH   1024

static const char* paths[] = { TEST_DATA_DIR_1, TEST_DATA_DIR_2, TEST_DATA_DIR_3, TEST_DATA_DIR_4 };
static char        path_buffer[64];
static char        stdio_buffer[1024];

int                c8_init_graphics(void) { return 0; }

char*              get_path(const char* filename) {
    for (int i = 0; i < 4; i++) {
        sprintf(path_buffer, "%s%s", paths[i], filename);
        if (access(path_buffer, F_OK) == 0) {
            return path_buffer;
        }
    }
    return NULL;
}
