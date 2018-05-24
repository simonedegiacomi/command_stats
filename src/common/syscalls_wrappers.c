#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#include "syscalls_wrappers.h"
#include "common.h"


int my_open(const char *pathname, int flags) {
    int fd = open(pathname, flags);
    if (fd == -1) {
        syscall_fail(pathname);
    }
    return fd;
}


ssize_t my_write(int fd, void *buffer, size_t count) {
    ssize_t res = write(fd, buffer, count);
    if (res == -1) {
        program_fail("Can't write to fd");
    }
    return res;
}


int my_close(int fd) {
    int res = close(fd);
    if (res == -1) {
        syscall_fail("Can't close fd");
    }
    return res;
}


int my_unlink(const char *pathname) {
    int res = unlink(pathname);
    if (res == -1) {
        syscall_fail(pathname);
    }
    return res;
}


int my_mkfifo(const char *pathname, mode_t mode) {
    int res = mkfifo(pathname, mode);
    if (res == -1) {
        syscall_fail(pathname);
    }
    return res;
}


int my_kill(pid_t pid, int sig) {
    int res = kill(pid, sig);
    if (res == -1) {
        syscall_fail("Can't send signal");
    }
    return res;
}

/* ***** MESSAGE QUEUES ***** */

key_t my_ftok(const char *pathname, int proj_id) {
    key_t message_queue_key = ftok(pathname, proj_id);
    if (message_queue_key == -1) {
        printf("%s, %d, %d\n", pathname, proj_id, message_queue_key);
        syscall_fail("Can't key for message queue");
    }
    return message_queue_key;
}

int my_msgget(key_t key, int msgflg) {
    int message_queue_id = msgget(key, msgflg);
    if (message_queue_id == -1) {
        syscall_fail("Can't create message queue");
    }
    return message_queue_id;
}

int my_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
    int res = msgsnd(msqid, msgp, msgsz, msgflg);
    if (res == -1) {
        syscall_fail("Can't send message to message queue");
    }
    return res;
}

int my_msgctl(int msqid, int cmd, struct msqid_ds *buf) {
    int res = msgctl(msqid, cmd, buf);
    if (res == -1) {
        syscall_fail("Can't destroy message queue");
    }
    return res;
}



