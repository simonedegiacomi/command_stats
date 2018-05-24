#include "../common/common.h"
#include "daemon_common.h"

void print_message(Message message) {
	print_log("\tmessage_type: %ld\n", message.message_type);
	print_log("\tbooking_info.pid: %d\n", message.booking_info.pid);
	print_log("\tbooking_info.log_path: %s\n", message.booking_info.log_path);
}

