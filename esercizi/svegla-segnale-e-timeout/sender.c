#include <stdio.h>
#include <signal.h>

int main (int argc, char *argv[]) {

	int desinationPid;
	printf("[SENDER] Dimmi il pid a cui inviare il segnale: ");
	scanf("%d", &desinationPid);

	kill(desinationPid, SIGCONT);

	return 0;
}

