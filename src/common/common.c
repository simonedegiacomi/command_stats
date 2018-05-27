#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <memory.h>
#include "common.h"
#include "syscalls_wrappers.h"

#define COPY_BUFFER     1024

/**
 * Copies chunk data until the from stream is closed from the other side.
 * NOTE: This function doesn't close any of the two file descriptors.
 * @param from
 * @param to
 */
void copy_stream (int from, int to) {
    char buffer[COPY_BUFFER];
    ssize_t read_res;
    do {
        read_res = read(from, buffer, sizeof(buffer));
        if (read_res > 0) {
            my_write(to, buffer, (size_t) read_res);
        }
    } while (read_res > 0);
}

/** Logging functions */
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

/** End of logging functions */


/** Time utilities */
void print_timespec(struct timespec time, FILE *out) {
    double ms = (time.tv_nsec / 1000000.0);

    int h = time.tv_sec / (60 * 60);
    int m = (time.tv_sec - (h * 60 * 60)) / 60;
    int s = (time.tv_sec - (h * 60 * 60) - (m * 60));

    fprintf(out, "%d h %d m %d s %f ms", h, m, s, ms);
}


void print_timeval(struct timeval time, FILE *out) {
    double ms = (time.tv_usec / 1000.0);

    int h = time.tv_sec / (60 * 60);
    int m = (time.tv_sec - (h * 60 * 60)) / 60;
    int s = (time.tv_sec - (h * 60 * 60) - (m * 60));

    fprintf(out, "%d h %d m %d s %f ms", h, m, s, ms);
}


struct timespec get_current_time () {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return res;
}

/** End of time utilities */
