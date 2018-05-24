#ifndef SYSCALLS_WRAPPERS_H
#define SYSCALLS_WRAPPERS_H


#include <sys/types.h>
#include <sys/msg.h>

void syscall_fail(const char *message);
int my_open(const char *pathname, int flags);
char *my_read(int fd);
ssize_t my_write(int fd, void *buffer, size_t count);

/* ***** MESSAGE QUEUES ***** */
key_t my_ftok(const char *pathname, int proj_id);
int my_msgget(key_t key, int msgflg);
int my_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t my_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
int my_msgctl(int msqid, int cmd, struct msqid_ds *buf);

#endif
