#ifndef DAEMON_COMMON_H
#define DAEMON_COMMON_H


#include <unistd.h>

#define MESSAGE_TYPE 1
#define MAX_LOG_PATH 4098

typedef struct BookingInfo {
	int pid;
	char log_path[MAX_LOG_PATH];
} BookingInfo;

typedef struct Message {
	long message_type;
	BookingInfo booking_info;
} Message;


extern const char *lock_file_path;
extern const char *stats_fifo_path;


void print_message			(Message message);
int get_message_queue_id 	(pid_t daemon_pid);

#endif
