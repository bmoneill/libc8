#include <stdio.h>
#include <unistd.h>

#define WRITE_TO_STDIN(s)                                                                          \
    fflush(stdin);                                                                                 \
    int fds[2];                                                                                    \
    pipe(fds);                                                                                     \
    write(fds[1], s, strlen(s));                                                                   \
    close(fds[1]);                                                                                 \
    dup2(fds[0], STDIN_FILENO);                                                                    \
    close(fds[0]);                                                                                 \
    clearerr(stdin);

#define REDIRECT_STDOUT                                                                            \
    freopen("/dev/null", "a", stdout);                                                             \
    setbuf(stdout, stdout_buffer);
#define RESTORE_STDOUT freopen("/dev/tty", "w", stdout);

#define TEST_DATA_DIR_1 "test/data/"
#define TEST_DATA_DIR_2 "data/"
#define TEST_DATA_DIR_3 "../test/data/"
#define TEST_DATA_DIR_4 "../../test/data/"
#define BUFFER_LENGTH   1024

static const char* paths[] = { TEST_DATA_DIR_1, TEST_DATA_DIR_2, TEST_DATA_DIR_3, TEST_DATA_DIR_4 };
static char        path_buffer[64];
static char        stdout_buffer[1024];

char*              get_path(const char* filename) {
    for (int i = 0; i < 4; i++) {
        sprintf(path_buffer, "%s%s", paths[i], filename);
        if (access(path_buffer, F_OK) == 0) {
            return path_buffer;
        }
    }
    return NULL;
}
