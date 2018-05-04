#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<string.h>

struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[100];    /* message data */
};


int main (int argc, char *argv[]) {
	key_t key = ftok("/Volumes/RamDisk/a", 5);
	int queue_id = msgget(key, 0777 | IPC_CREAT);

	printf("La mia q id: %d\n", queue_id);

	
	struct msgbuf message;
	message.mtype = 2;
	strcpy(message.mtext, "prova");

	int res = msgsnd(queue_id, &message, sizeof(message), 1);
	printf("%d\n", res);

	return 0;
}

