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
#include "daemon.h"

const int MAX_ATTEMPTS = 3;


/** Private functions declaration */

void initialize_booking_confirmation_receiver();
void send_booking (const char *log_path);
void wait_booking_confirmation ();
void finalize_booking_confirmation_receiver();
int open_booked_pipe();

BOOL try_to_read_daemon_pid(int *daemon_pid_dst);
/**  End of private functions declaration */


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


int book_and_obtain_log_fd(const char *log_path) {
    initialize_booking_confirmation_receiver();

    send_booking(log_path);
    wait_booking_confirmation();

    finalize_booking_confirmation_receiver();
    return open_booked_pipe();
}

void initialize_booking_confirmation_receiver(){
    signal(SIGCONT, on_continue_signal);
}

void send_booking (const char *log_path) {
    pid_t daemon_pid = read_daemon_pid();

    int message_queue_id = get_message_queue_id(daemon_pid);

    Message message = {
        .message_type = MESSAGE_TYPE,
        .booking_info = {
            .pid = getpid()
        }
    };
    // NOTE: Memory MUST be continue, we cannot use strdup or copy the pointer!
    strcpy(message.booking_info.log_path, log_path);

    my_msgsnd(message_queue_id, &message, sizeof(BookingInfo), 0);

    print_log("[DAEMON_SOCKET] Sent message:\n");
    print_message(message);
}

void wait_booking_confirmation () {
    print_log("[DAEMON_SOCKET] Waiting for signal from daemon...");
    while (!continue_signal) {	//https://www.gnu.org/software/libc/manual/html_node/Pause-Problems.html#Pause-Problems
        sleep(1);
        //pause();
    }
    print_log(" finally!\n");
}

void finalize_booking_confirmation_receiver(){
    signal(SIGCONT, SIG_DFL);
}

int open_booked_pipe(){
    return my_open(stats_fifo_path, O_WRONLY);
}


pid_t read_daemon_pid() {

	int attempts;
    BOOL success;
    pid_t daemon_pid = - 1;

    for (attempts = 0; attempts < MAX_ATTEMPTS && !success; attempts++) {
        success = try_to_read_daemon_pid(&daemon_pid);

        BOOL last_attempts = attempts == (MAX_ATTEMPTS - 1);
        if (!success && !last_attempts) {
            print_log("[RUN] Can't read daemon pid, restarting daemon...\n");
            start_daemon();
        }
    }

    if (!success) {
        program_fail("[RUN] Can't read daemon pid\n");
    } else {
        print_log("[DAEMON_SOCKET] Daemon is running with pid %d\n", daemon_pid);
    }
    return daemon_pid;
}

BOOL try_to_read_daemon_pid(int *daemon_pid_dst) {
    int daemon_pid = -1;
    struct flock lock = get_flock_for_lock();
    int attempts;

    for (attempts = 0; attempts < MAX_ATTEMPTS && daemon_pid == -1; attempts++) {

        int lock_fd = open(lock_file_path, O_RDONLY);

        if (lock_fd != -1) {
            lock.l_pid = -1;
            fcntl(lock_fd, F_GETLK, &lock);
            daemon_pid = lock.l_pid;
            my_close(lock_fd);
        }

        if (daemon_pid == -1) {
            print_log("[RUN] Can't read daemon pid, retry after 1s\n");
            sleep(1);
        }
    }

    if (daemon_pid == -1) {
        return FALSE;
    } else {
        *daemon_pid_dst = daemon_pid;
        return TRUE;
    }
}

