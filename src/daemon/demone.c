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

#define MESSAGE_TYPE 1

typedef struct BookingInfo {
	int pid;
	char log_path[4096];
} BookingInfo;

typedef struct Message {
	long message_type;
	BookingInfo booking_info;
} Message;

const char *lock_file_path = "/tmp/SO_project.lock";
const char *stats_fifo_path = "/tmp/SO_stats_fifo";

pid_t read_daemon_pid() {
	int lock_file_fd = my_open(lock_file_path, O_RDONLY);
	
	pid_t daemon_pid;
	if (read(lock_file_fd, &daemon_pid, sizeof(daemon_pid)) < 1) {
		syscall_fail("[DAEMON] Unable to read daemon pid from lock file\n");
	}
	
	close(lock_file_fd);
	
	return daemon_pid;
}

void write_daemon_pid(int lock_file_fd) {
	pid_t daemon_pid = getpid();
	write(lock_file_fd, &daemon_pid, sizeof(daemon_pid));
	print_log("[DAEMON] Daemon pid: %d\n", daemon_pid);
}

void print_message (Message message) {
	print_log("\tmessage_type: %ld\n", message.message_type);
	print_log("\tbooking_info.pid: %d\n", message.booking_info.pid);
	print_log("\tbooking_info.log_path: %s\n", message.booking_info.log_path);
}

void finalize_daemon(int message_queue_id, const char *stats_fifo_path, const char *lock_file_path) {
	my_msgctl(message_queue_id, IPC_RMID, NULL);
	my_unlink(stats_fifo_path);
	my_unlink(lock_file_path);
}


int interrupt = 0;
void signal_handler(int sig) {
	interrupt = 1;
}


int main()
{
	enable_logging();
	
	int lock_file_fd = open(lock_file_path, O_CREAT|O_EXCL|O_RDWR, 0666);
	
	if (lock_file_fd < 0) {
		pid_t daemon_pid = read_daemon_pid();
		
		//program_fail("Daemon is already running with PID %d.\n", daemon_pid);
		//fprintf(stderr, "Daemon is already running with PID %d.\n", daemon_pid);
		print_log("[DAEMON] Daemon is already running with pid %d\n", daemon_pid);
		
		
		key_t message_queue_key = my_ftok(lock_file_path, daemon_pid);
		int message_queue_id = my_msgget(message_queue_key, 0666 | IPC_CREAT);
		
		char *log_path = "percorso file log";
		Message message;
		message.message_type = MESSAGE_TYPE;
		message.booking_info.pid = getpid();
		strcpy(message.booking_info.log_path, log_path);
		
		my_msgsnd(message_queue_id, &message, sizeof(BookingInfo), 0);
		
		print_log("[DAEMON] Sent message:\n");
		print_message(message);
		
		
		signal(SIGCONT, signal_handler);
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
	
		
		
		exit(1);
	}
	
	// TODO: se il run va a leggere il contenuto del file in questo momento?
	
	write_daemon_pid(lock_file_fd);
	close(lock_file_fd);
	
	
	key_t message_queue_key = my_ftok(lock_file_path, getpid());
	int message_queue_id = my_msgget(message_queue_key, 0666 | IPC_CREAT);
	
	Message message;
	my_msgrcv(message_queue_id, &message, sizeof(BookingInfo), MESSAGE_TYPE, 0);
	
	print_log("[DAEMON] Received message:\n");
	print_message(message);
	
	
	
	
	
	sleep(2);
	puts("Invio segnale...");
	kill(message.booking_info.pid, SIGCONT);
	
	
	
	
	mkfifo(stats_fifo_path, 0666);
	perror("ERRORE");
	char statistics[80];

	int stats_fifo_fd = open(stats_fifo_path, O_RDONLY);
	perror("ERRORE");
	read(stats_fifo_fd, statistics, sizeof(statistics));
	perror("ERRORE");
	printf("Statistiche ricevute: %s\n", statistics);
	close(stats_fifo_fd);
	
	
	/* here the real code of the app*/
	int i;
	for (i = 5; i > 0; i--) {
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	printf("\n");
	
	/* end of the app */
	finalize_daemon(message_queue_id, stats_fifo_path, lock_file_path);
	return 0;
}
