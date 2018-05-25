#ifndef DAEMON_SOCKET_H
#define DAEMON_SOCKET_H


struct flock get_flock_for_lock();

// TODO: Explain differences and change names
pid_t   read_daemon_pid();
BOOL try_to_read_daemon_pid(int *daemon_pid_dst);
int     book_and_obtain_log_fd(const char *log_path);

#endif
