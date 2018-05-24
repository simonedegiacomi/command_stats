#ifndef DAEMON_COMMON_H
#define DAEMON_COMMON_H


#define MESSAGE_TYPE 1

typedef struct BookingInfo {
	int pid;
	char log_path[4096];
} BookingInfo;

typedef struct Message {
	long message_type;
	BookingInfo booking_info;
} Message;


static const char *lock_file_path = "/tmp/SO_project.lock";
static const char *stats_fifo_path = "/tmp/SO_stats_fifo";


void print_message(Message message);

#endif
