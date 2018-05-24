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

const int MAX_ATTEMPTS = 3;

pid_t read_daemon_pid();


BOOL continue_signal = FALSE;
void on_continue_signal(int sig) {
	continue_signal = TRUE;
}


struct flock get_flock_for_lock() {
    return (struct flock){
        .l_start    = 0,
        .l_len      = 0,
        .l_type     = F_WRLCK,
        .l_whence   = SEEK_SET
    };
}


int obtain_log_fd(const char *log_path) {
	pid_t daemon_pid = read_daemon_pid();
	print_log("[DAEMON_SOCKET] Daemon is running with pid %d\n", daemon_pid);

	int message_queue_id = get_message_queue_id(daemon_pid);
	
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


	int stats_fifo_fd = my_open(stats_fifo_path, O_WRONLY);
	
	return stats_fifo_fd;
}


pid_t read_daemon_pid() {

	int attempts;
    struct flock lock = get_flock_for_lock();
    pid_t daemon_pid = - 1;

    for (attempts = 0; attempts < MAX_ATTEMPTS && daemon_pid == -1; attempts++) {

        int lock_fd = open(lock_file_path, O_RDONLY);

        if (lock_fd != -1) {
            lock.l_pid = -1;
            int res = fcntl(lock_fd, F_GETLK, &lock);
            daemon_pid = lock.l_pid;
            my_close(lock_fd);
        }

        if (daemon_pid == -1) {
            print_log("[RUN] Can't read daemon pid, retry after 1s\n");
            sleep(1);
        }
	}

	if (daemon_pid == -1) {
		program_fail("[RUN] Can't read daemon pid\n");
	}
	
	return daemon_pid;
}

