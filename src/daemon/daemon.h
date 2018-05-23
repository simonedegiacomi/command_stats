#ifndef DAEMON_H
#define DAEMON_H

void start_daemon();

#define MESSAGE_TYPE 1

typedef struct BookingInfo {
	int pid;
	char log_path[4096];
} BookingInfo;

typedef struct Message {
	long message_type;
	BookingInfo booking_info;
} Message;


#endif
