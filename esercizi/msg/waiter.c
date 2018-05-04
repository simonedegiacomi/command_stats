#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>


struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[100];    /* message data */
};


int main (int argc, char *argv[]) {
	key_t key = ftok("/Volumes/RamDisk/a", 5);
	int queue_id = msgget(key, 0777 | IPC_CREAT);

	printf("Sto aspettando (Q id: %d)\n", queue_id);

	struct msgbuf buffer;
	msgrcv(queue_id, &buffer, sizeof(buffer), 2, 0);

	printf("Ho ricevuto: %s\n", buffer.mtext);

	msgctl(queue_id, IPC_RMID, NULL);
	

	return 0;
}




