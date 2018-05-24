#ifndef DAEMON_SOCKET_H
#define DAEMON_SOCKET_H

pid_t   read_daemon_pid();
int     obtain_log_fd(const char *log_path);

#endif
