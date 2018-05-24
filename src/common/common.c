#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include "common.h"
#include "syscalls_wrappers.h"

long get_current_time () {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return res.tv_sec;
}

static BOOL log_enabled = FALSE;

void enable_logging () {
    log_enabled = TRUE;
}

BOOL is_logging_enabled() {
	return log_enabled;
}

void print_log(const char *format, ...) {
    if (log_enabled) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
    }
}

void program_fail(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    exit(1);
}

void syscall_fail(const char *message) {
	perror(message);
	exit(1);
}

/**
 * Copies chunk data until the from stream is closed from the other side.
 * NOTE: This function doesn't close any of the two file descriptors.
 * @param from
 * @param to
 */
void copy_stream (int from, int to) {
    const int BUFFER_SIZE = 1024;

    char buffer[BUFFER_SIZE];
    ssize_t read_res;
    do {
        read_res = read(from, buffer, sizeof(buffer));
        if (read_res > 0) {
            my_write(to, buffer, (size_t) read_res);
        }
    } while (read_res > 0);
}
