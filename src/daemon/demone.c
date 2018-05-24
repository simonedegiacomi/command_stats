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
#include "daemon_socket.h"



#include "daemon_common.h"




int main()
{
	enable_logging();
	
	start_daemon();
	
	char *log_path = "/tmp/percorso_file_log.txt";
	int log_fd = obtain_log_fd(log_path);
	
	char *statistics = "statisticheee";
	printf("Statistiche inviate: %s\n", statistics);
	write(log_fd, statistics, strlen(statistics)+1);
	
	my_close(log_fd);
	
	
	return 0;
}
