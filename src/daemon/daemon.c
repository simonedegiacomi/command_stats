#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <signal.h>
#include <errno.h>

#include "../common/common.h"
#include "../common/syscalls_wrappers.h"
#include "daemon_common.h"
#include "daemon.h"
#include "daemonize.h"
#include "daemon_socket.h"


void run_daemon_main();
int initialize_daemon();
void finalize_daemon(int message_queue_id);

BOOL interrupt_signal = FALSE;
void on_interrupt_signal(int sig) {
	interrupt_signal = TRUE;
	print_log("[DAEMON] SIGINT or SIGTERM received\n");
}


void start_daemon() {
    DaemonizeResult res = daemonize();
    if (res == FAILED) {
        program_fail("Can't start daemon");
    } else if (res == SUCCESS_DAEMON) {
        run_daemon_main();
    }
}

/**
 *
 * @return
 */
void acquire_lock_or_exit() {
    int lock_fd = open(lock_file_path, O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) {
        syscall_fail(lock_file_path);
    }


    struct flock lock = get_flock_for_lock();
    if (fcntl(lock_fd, F_SETLK, &lock) == -1) {
        print_log("[DAEMON] Another daemon is already running, exiting");
        exit(0);
    }
}

void run_daemon_main() {
    int message_queue_id = initialize_daemon();

    while (!interrupt_signal) {
		Message message;
		ssize_t res = msgrcv(message_queue_id, &message, sizeof(BookingInfo), MESSAGE_TYPE, 0);
        if (res == -1 && errno != EINTR) {
            syscall_fail("Can't receive message from message queue");
        }

        if (res > 0) {
            print_log("[DAEMON] Received message:\n");
            print_message(message);


            print_log("[DAEMON] Sending SIGCONT to %d\n",
                      message.booking_info.pid);
            kill(message.booking_info.pid, SIGCONT);


            int stats_fifo_fd = my_open(stats_fifo_path, O_RDONLY);

            int stats_log_fd = open(message.booking_info.log_path,
                                    O_CREAT | O_APPEND | O_WRONLY, 0666);

            copy_stream(stats_fifo_fd, stats_log_fd);

            my_close(stats_log_fd);
            my_close(stats_fifo_fd);
        }
	}


	finalize_daemon(message_queue_id);
	exit(0);
}

/**
 * Tries to acquire exclusive lock on the lock file and exit if the lock file is
 * already locked.
 * If the lock is successfully acquired, writes the daemon pid into the daemon
 * pid file and initializes the daemon.
 * @return message queue id from which read BookingInfo from run clients.
 */
int initialize_daemon() {
    acquire_lock_or_exit();

    int message_queue_id = get_message_queue_id(getpid());

    signal(SIGINT, on_interrupt_signal);
    signal(SIGTERM, on_interrupt_signal);

    unlink(stats_fifo_path);
    my_mkfifo(stats_fifo_path, 0666);
    return message_queue_id;
}


void finalize_daemon(int message_queue_id) {
    print_log("[DAEMON] Exiting...\n");
	my_msgctl(message_queue_id, IPC_RMID, NULL);
	my_unlink(stats_fifo_path);
    my_unlink(lock_file_path);
}


void stop_daemon() {
	my_kill(read_daemon_pid(), SIGINT);
}

