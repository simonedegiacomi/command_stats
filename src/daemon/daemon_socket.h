#ifndef DAEMON_SOCKET_H
#define DAEMON_SOCKET_H


struct flock get_flock_for_lock();

pid_t   read_daemon_pid();
int     book_and_obtain_log_fd(const char *log_path);

#endif
