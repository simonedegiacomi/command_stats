#ifndef COMMON_H
#define COMMON_H

#include <sys/time.h>
#include <stdio.h>
#include <time.h>

#define BOOL    char
#define TRUE    1
#define FALSE   0

// pipe file descriptor indexes
#define WRITE_INTO_PIPE     1
#define READ_FROM_PIPE      0

void copy_stream                    (int from, int to);

/** Logging functions */
void enable_logging                 ();
BOOL is_logging_enabled             ();
void print_log                      (const char *format, ...);
void program_fail                   (const char *format, ...);
void syscall_fail                   (const char *message);
/** End of logging functions */


/** Time utilities */
void print_timespec                 (struct timespec time, FILE *out);
void print_timeval                  (struct timeval time, FILE *out);
struct timespec get_current_time    ();
/** End of time utilities */

#endif
