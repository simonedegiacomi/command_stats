#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <signal.h>

#include "../common/common.h"
#include "../common/syscalls_wrappers.h"
#include "daemon.h"


static const char *lock_file_path = "/tmp/SO_project.lock";
static const char *stats_fifo_path = "/tmp/SO_stats_fifo";

pid_t read_daemon_pid() {
	int lock_file_fd = my_open(lock_file_path, O_RDONLY);
	
	pid_t daemon_pid;
	if (read(lock_file_fd, &daemon_pid, sizeof(daemon_pid)) < 1) {
		syscall_fail("[DAEMON] Unable to read daemon pid from lock file\n");
	}
	
	close(lock_file_fd);
	
	return daemon_pid;
}



int interrupt = 0;
void signal_handler(int sig) {
	interrupt = 1;
}


int main()
{
	enable_logging();
	
	start_daemon();
	
	
	
	
	pid_t daemon_pid = read_daemon_pid();
	
	signal(SIGCONT, signal_handler);
	//program_fail("Daemon is already running with PID %d.\n", daemon_pid);
	//fprintf(stderr, "Daemon is already running with PID %d.\n", daemon_pid);
	print_log("[DAEMON] Daemon is already running with pid %d\n", daemon_pid);
	
	
	key_t message_queue_key = my_ftok(lock_file_path, daemon_pid);
	int message_queue_id = my_msgget(message_queue_key, 0666 | IPC_CREAT);
	
	char *log_path = "/tmp/percorso_file_log.txt";
	Message message;
	message.message_type = MESSAGE_TYPE;
	message.booking_info.pid = getpid();
	strcpy(message.booking_info.log_path, log_path);
	
	my_msgsnd(message_queue_id, &message, sizeof(BookingInfo), 0);
	
	print_log("[DAEMON] Sent message:\n");
	print_message(message);
	
	
	puts("Mi metto in pausa...");
	while (!interrupt) {	//https://www.gnu.org/software/libc/manual/html_node/Pause-Problems.html#Pause-Problems
		sleep(1);
		//pause();
	}
	puts("CONTINUO\n");
	
	
	
	//if (mkfifo(stats_fifo_path, 0666)<0)	{perror("fifo");}		// https://stackoverflow.com/a/2789967 (va chiamato solo una volta)
	char *statistics = "statisticheee";
	printf("Statistiche inviate: %s\n", statistics);

	int stats_fifo_fd = open(stats_fifo_path, O_WRONLY);
perror("ERRORE");
	write(stats_fifo_fd, statistics, strlen(statistics)+1);
	close(stats_fifo_fd);

	
	
	return 0;
}
