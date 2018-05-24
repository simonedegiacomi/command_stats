#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <unistd.h>

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
		program_fail("Can't write to output or receiving node, exiting.\n");
	}
	return res;
}


int my_close(int fd) {
	int ret_val = close(fd);
	if (ret_val == -1) {
		syscall_fail("Error while closing fd");
	}
	return ret_val;
}


int my_unlink(const char *pathname) {
	int ret_val = unlink(pathname);
	if (ret_val == -1) {
		syscall_fail(pathname);
	}
	return ret_val;
}


int my_mkfifo(const char *pathname, mode_t mode) {
	int ret_val = mkfifo(pathname, mode);
	if (ret_val == -1) {
		syscall_fail(pathname);
	}
	return ret_val;
}


/* ***** MESSAGE QUEUES ***** */

key_t my_ftok(const char *pathname, int proj_id) {
	key_t message_queue_key = ftok(pathname, proj_id);
	if (message_queue_key == -1) {
		printf("%s, %d, %d\n", pathname, proj_id, message_queue_key);
		syscall_fail("[DAEMON] Error while generating key for message queue");
	}
	return message_queue_key;
}

int my_msgget(key_t key, int msgflg) {
	int message_queue_id = msgget(key, msgflg);
	if (message_queue_id == -1) {
		syscall_fail("[DAEMON] Error while creating message queue");
	}
	return message_queue_id;
}

int my_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
	int ret_val = msgsnd(msqid, msgp, msgsz, msgflg);
	if (ret_val == -1) {
		syscall_fail("[DAEMON] Error while sending message");
	}
	return ret_val;
}

ssize_t my_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
	int ret_val = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
	if (ret_val == -1) {
		syscall_fail("[DAEMON] Error while receiving message");
	}
	return ret_val;
}

int my_msgctl(int msqid, int cmd, struct msqid_ds *buf) {
	int ret_val = msgctl(msqid, cmd, buf);
	if (ret_val == -1) {
		syscall_fail("[DAEMON] Error while destroying message queue");
	}
	return ret_val;
}



