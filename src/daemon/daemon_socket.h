#ifndef DAEMON_SOCKET_H
#define DAEMON_SOCKET_H

#include <signal.h>

#define DAEMON_RESPONSE_OK_NEW_FILE         SIGCONT
#define DAEMON_RESPONSE_OK_FILE_EXISTS      SIGUSR1
#define DAEMON_RESPONSE_ERROR_CANT_WRITE    SIGUSR2


struct flock    get_flock_for_lock();


pid_t           read_daemon_pid();
BOOL            try_to_read_daemon_pid(int *daemon_pid_dst);
int             book_and_obtain_log_fd(const char *log_path);

#endif
