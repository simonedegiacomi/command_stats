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
#include "daemon_common.h"
#include "daemon_socket.h"

pid_t read_daemon_pid();


BOOL continue_signal = FALSE;
void on_continue_signal(int sig) {
	continue_signal = TRUE;
}


int obtain_log_fd(const char *log_path) {
	pid_t daemon_pid = read_daemon_pid();
	print_log("[DAEMON_SOCKET] Daemon is already running with pid %d\n", daemon_pid);
	
	key_t message_queue_key = my_ftok(lock_file_path, daemon_pid);
	int message_queue_id = my_msgget(message_queue_key, 0666 | IPC_CREAT);
	
	signal(SIGCONT, on_continue_signal);
	
	Message message;
	message.message_type = MESSAGE_TYPE;
	message.booking_info.pid = getpid();
	strcpy(message.booking_info.log_path, log_path);
	
	my_msgsnd(message_queue_id, &message, sizeof(BookingInfo), 0);
	
	print_log("[DAEMON_SOCKET] Sent message:\n");
	print_message(message);
	
	
	print_log("[DAEMON_SOCKET] Waiting for signal from daemon...");
	while (!continue_signal) {	//https://www.gnu.org/software/libc/manual/html_node/Pause-Problems.html#Pause-Problems
		sleep(1);
		//pause();
	}
	print_log(" finally!\n");
	
	
	
	char *statistics = "statisticheee";
	printf("Statistiche inviate: %s\n", statistics);

	int stats_fifo_fd = my_open(stats_fifo_path, O_WRONLY);
	
	return stats_fifo_fd;
}


pid_t read_daemon_pid() {
	int lock_file_fd = my_open(lock_file_path, O_RDONLY);
	
	pid_t daemon_pid;
	if (read(lock_file_fd, &daemon_pid, sizeof(daemon_pid)) < 1) {
		syscall_fail("[DAEMON] Unable to read daemon pid from lock file\n");
	}
	
	my_close(lock_file_fd);
	
	return daemon_pid;
}

