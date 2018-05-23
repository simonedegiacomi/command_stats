#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "syscalls_wrappers.h"


void syscall_fail(const char *message) {
	perror(message);
	exit(1);
}


int my_open(const char *pathname, int flags) {
	int fd = open(pathname, flags);
	if (fd < 1) {
		syscall_fail(pathname);
	}
	return fd;
}
/*
int my_open(const char *pathname, int flags, mode_t mode) {
	int fd = open(pathname, flags, mode);
	if (fd < 1) {
		syscall_fail(pathname);
	}
	return fd;
}
*/

char *my_read(int fd) {
	const int MAX_BUF_SIZE = 10;
	char *str = (char*) malloc(MAX_BUF_SIZE * sizeof(char));	// TODO: sostituire con wrapper nostro? (restituisce NULL in caso di errore)
	if (read(fd, str, MAX_BUF_SIZE) < 1) {
		syscall_fail("Impossibile leggere il PID del daemon");
	}
	return str;
}



/* ***** MESSAGE QUEUES ***** */

key_t my_ftok(const char *pathname, int proj_id) {
	key_t message_queue_key = ftok(pathname, proj_id);
	if (message_queue_key < 0) {
		syscall_fail("Errore durante la generazione della chiave per la message queue");
	}
	return message_queue_key;
}

int my_msgget(key_t key, int msgflg) {
	int message_queue_id = msgget(key, msgflg);
	if (message_queue_id < 0) {
		syscall_fail("Errore durante la creazione della message queue");
	}
	return message_queue_id;
}

int my_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
	int ret_val = msgsnd(msqid, msgp, msgsz, msgflg);
	if (ret_val < 0) {
		syscall_fail("Errore durante l'invio del messaggio");
	}
	return ret_val;
}

ssize_t my_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
	int ret_val = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
	if (ret_val < 0) {
		syscall_fail("Errore durante la ricezione del messaggio");
	}
	return ret_val;
}

int my_msgctl(int msqid, int cmd, struct msqid_ds *buf) {
	int ret_val = msgctl(msqid, cmd, buf);
	if (ret_val < 0) {
		syscall_fail("Errore durante l'eliminazione della message queue");
	}
	return ret_val;
}




