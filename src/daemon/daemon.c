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
#include "daemonize.h"


void run_daemon_main();
void write_daemon_pid(int lock_file_fd);
void finalize_daemon(int message_queue_id);



void print_message (Message message) {
	print_log("\tmessage_type: %ld\n", message.message_type);
	print_log("\tbooking_info.pid: %d\n", message.booking_info.pid);
	print_log("\tbooking_info.log_path: %s\n", message.booking_info.log_path);
}


static const char *lock_file_path = "/tmp/SO_project.lock";
static const char *stats_fifo_path = "/tmp/SO_stats_fifo";


BOOL interrupt_signal = FALSE;
void on_interrupt_signal(int sig) {
	interrupt_signal = TRUE;
}


void start_daemon() {
	int lock_file_fd = open(lock_file_path, O_CREAT|O_EXCL|O_RDWR, 0666);
	
	if (lock_file_fd != -1) {
		if (daemonize() == 1) {
			close(lock_file_fd);
			return;
		}
		
		// TODO: se il run va a leggere il contenuto del file in questo momento?
		write_daemon_pid(lock_file_fd);
		close(lock_file_fd);
		
		run_daemon_main();
	}
}

void run_daemon_main() {
	key_t message_queue_key = my_ftok(lock_file_path, getpid());
	int message_queue_id = my_msgget(message_queue_key, 0666 | IPC_CREAT);
	
	signal(SIGINT, on_interrupt_signal);
	
	my_mkfifo(stats_fifo_path, 0666);
	
	while (!interrupt_signal) {
		Message message;
		my_msgrcv(message_queue_id, &message, sizeof(BookingInfo), MESSAGE_TYPE, 0);
		
		print_log("[DAEMON] Received message:\n");
		print_message(message);
		
		
		print_log("[DAEMON] Sending SIGCONT to %d\n", message.booking_info.pid);
		kill(message.booking_info.pid, SIGCONT);


		int stats_fifo_fd = my_open(stats_fifo_path, O_RDONLY);
		
		int stats_log_fd = open(message.booking_info.log_path, O_CREAT | O_APPEND | O_WRONLY, 0666);
		char buffer[1024];
		ssize_t read_res;
		do {
			read_res = read(stats_fifo_fd, buffer, sizeof(buffer));
			if (read_res > 0) {
				my_write(stats_log_fd, buffer, (size_t)read_res);
			}
		} while (read_res > 0);
		
		my_close(stats_log_fd);
		my_close(stats_fifo_fd);
	}

	/* end of the app */
	finalize_daemon(message_queue_id);
	
	exit(0);
}

void write_daemon_pid(int lock_file_fd) {
	pid_t daemon_pid = getpid();
	write(lock_file_fd, &daemon_pid, sizeof(daemon_pid));
	print_log("[DAEMON] Daemon pid: %d\n", daemon_pid);
}

void finalize_daemon(int message_queue_id) {
	my_msgctl(message_queue_id, IPC_RMID, NULL);
	my_unlink(stats_fifo_path);
	my_unlink(lock_file_path);
}


