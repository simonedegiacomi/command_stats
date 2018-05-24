#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include "common.h"


long get_current_time () {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return res.tv_sec;
}

static BOOL log_enabled = FALSE;

void enable_logging () {
    log_enabled = TRUE;
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


