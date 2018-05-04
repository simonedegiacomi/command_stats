#include <stdio.h>
#include <unistd.h>
#include <signal.h>


void on_signal (int signal) {
	switch (signal) {
		case SIGCONT:
			printf("[WAITER] ricevuto SIGCONT\n");
			break;
		case SIGALRM:
			printf("[WAITER] ricevuto SIGALRM\n");
			break;
		default:
			printf("[WAITER] Altro segnale\n");
	}
}

int main (int argc, char *argv[]) {

	printf("[WAITER] Il mio pid e': %d\n", getpid());

	signal(SIGCONT, on_signal);
	signal(SIGALRM, on_signal);
	alarm(5);
	pause();

	//printf("[WAITER] Ricevuto segnale %d da processo %d\n", signal, pidOfSender);

	return 0;
}

