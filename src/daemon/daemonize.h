#ifndef DAEMONIZE_H
#define DAEMONIZE_H

typedef enum DaemonizeResult {
    FAILED, SUCCESS_PARENT, SUCCESS_DAEMON
} DaemonizeResult ;

DaemonizeResult daemonize();

#endif
