#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <signal.h>
#include <errno.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/syscalls_wrappers.h"
#include "daemon_common.h"
#include "daemon.h"
#include "daemonize.h"
#include "daemon_socket.h"

/** Private functions declaration */
void    run_daemon_main         ();
void    acquire_lock_or_exit    ();

BOOL    file_exists             (const char *name);
void    handle_message          (Message *message);
int     initialize_daemon       ();
void    finalize_daemon         (int message_queue_id);
/** End of private functions declaration */

/**
 * Flag that becomes true when the signal to stop the daemon is received.
 */
BOOL interrupt_signal = FALSE;

/**
 * Handler of the signal to stop the daemon
 */
void on_interrupt_signal(int sig) {
	interrupt_signal = TRUE;
	print_log("[DAEMON] SIGINT or SIGTERM received\n");
}

/**
 * Starts the daemon and returns. If the daemon is already running the new
 * daemon will stop himself.
 */
void start_daemon() {
    DaemonizeResult res = daemonize();
    if (res == FAILED) {
        program_fail("Can't start daemon");
    } else if (res == SUCCESS_DAEMON) {
        run_daemon_main();
    }
}


/**
 * Main function of the daemon
 */
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

            handle_message(&message);
        }
	}


	finalize_daemon(message_queue_id);
	exit(0);
}


/**
 * Initializes the daemon. It performs the following operations:
 * 1. Try to acquire lock. If fail another daemon is already running;
 * 2. Setup queue to receive messages;
 * 3. Setup signal handlers;
 * 4. Create pipe to which the tool will write data;
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


/**
 * Tries to acquire exclusive lock on the lock file and exit if the lock file is
 * already locked.
 */
void acquire_lock_or_exit() {
    int lock_fd = open(lock_file_path, O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) {
        syscall_fail(lock_file_path);
    }


    struct flock lock = get_flock_for_lock();
    if (fcntl(lock_fd, F_SETLK, &lock) == -1) {
        print_log("[DAEMON] Another daemon is already running, exiting\n");
        exit(0);
    }
}

/**
 * Handles an incoming message from the tool, opening the file to which write the
 * log and notifying the tool.
 */
void handle_message (Message *message) {

    const char *tool_wd     = message->booking_info.log_path;
    size_t end              = strlen(tool_wd);
    const char *log_path    = &message->booking_info.log_path[end + 1];

    // Change dir into the tool directory (we need to this because the usare may
    // specify a relative path)
    chdir(tool_wd);

    // Check if the fiel exists and open it in write mode
    BOOL exists         = file_exists(log_path);
    int stats_log_fd    = open(log_path, O_CREAT | O_APPEND | O_WRONLY, 0666);


    if (stats_log_fd == -1) { // Can't open file
        print_log("[DAEMON] Sending SIGUSR2 to %d, can't open log file\n", message->booking_info.pid);
        kill(message->booking_info.pid, DAEMON_RESPONSE_ERROR_CANT_WRITE);
    } else {

        if (exists) {
            print_log("[DAEMON] Sending SIGUSR1 to %d, log file exists\n", message->booking_info.pid);
            kill(message->booking_info.pid, DAEMON_RESPONSE_OK_FILE_EXISTS);
        } else {
            print_log("[DAEMON] Sending SIGCONT to %d\n", message->booking_info.pid);
            kill(message->booking_info.pid, DAEMON_RESPONSE_OK_NEW_FILE);
        }

        // Open pipe to read data from tool
        int stats_fifo_fd = my_open(stats_fifo_path, O_RDONLY);

        // Copy data from tool to file
        copy_stream(stats_fifo_fd, stats_log_fd);

        my_close(stats_log_fd);
        my_close(stats_fifo_fd);
    }
}

BOOL file_exists (const char *name) {
    return access(name, F_OK) != -1;
}

void finalize_daemon(int message_queue_id) {
    print_log("[DAEMON] Exiting...\n");
	my_msgctl(message_queue_id, IPC_RMID, NULL);
	my_unlink(stats_fifo_path);
    my_unlink(lock_file_path);
}


void stop_daemon() {
    pid_t daemon_pid = -1;
    BOOL found = try_to_read_daemon_pid(&daemon_pid);
    if (found) {
        my_kill(read_daemon_pid(), SIGINT);
    } else {
        program_fail("[RUN] Daemon is not running\n");
    }
}

