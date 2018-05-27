#ifndef COMMON_H
#define COMMON_H

#include <sys/time.h>
#include <stdio.h>
#include <time.h>

#define BOOL char
#define TRUE 1
#define FALSE 0

// pipe file descriptor indexes
#define WRITE_INTO_PIPE 1
#define READ_FROM_PIPE 0


struct timespec get_current_time ();

void enable_logging();
BOOL is_logging_enabled();
void print_log(const char *format, ...);
void program_fail(const char *format, ...);
void syscall_fail(const char *message);


void copy_stream (int from, int to);

void print_timespec(struct timespec time, FILE *out);
void print_timeval(struct timeval time, FILE *out);

#endif
