#include "../common/common.h"
#include "daemon_common.h"
#include "../common/syscalls_wrappers.h"

void print_message(Message message) {
	print_log("\tmessage_type: %ld\n", message.message_type);
	print_log("\tbooking_info.pid: %d\n", message.booking_info.pid);
	print_log("\tbooking_info.log_path: %s\n", message.booking_info.log_path);
}


int get_message_queue_id (pid_t daemon_pid) {
    key_t message_queue_key = my_ftok(lock_file_path, daemon_pid);
    return my_msgget(message_queue_key, 0666 | IPC_CREAT);
}
