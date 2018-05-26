#ifndef SYSCALLS_WRAPPERS_H
#define SYSCALLS_WRAPPERS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>

int     my_open     (const char *pathname, int flags);

ssize_t my_write    (int fd, void *buffer, size_t bytes_to_write);
int     my_close    (int fd);
int     my_unlink   (const char *pathname);
int     my_mkfifo   (const char *pathname, mode_t mode);
int     my_kill     (pid_t pid, int sig);
FILE *  my_fdopen   (int fd, const char *mode);


/* ***** MESSAGE QUEUES ***** */
key_t   my_ftok     (const char *pathname, int proj_id);
int     my_msgget   (key_t key, int msgflg);
int     my_msgsnd   (int msqid, const void *msgp, size_t msgsz, int msgflg);
int     my_msgctl   (int msqid, int cmd, struct msqid_ds *buf);

#endif
