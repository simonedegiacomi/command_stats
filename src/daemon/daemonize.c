#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../common/common.h"
#include "daemonize.h"

DaemonizeResult daemonize() {
	pid_t pid, sid;
	int fd;

	/* already a daemon */
	if (getppid() == 1) {
		return FAILED;
	}

	pid = fork();
	if (pid < 0) {
		program_fail("[DAEMON] Can't fork first time");
	}
	if (pid > 0) {
		return SUCCESS_PARENT;
	} // keep only child

	sid = setsid();
	if (sid < 0) {
		program_fail("[DAEMON] Can't create a new session");
	} // detach session

	if (chdir("/") == -1) {
		program_fail("[DAEMON] Can't change dir");
	} // change pwd

	pid = fork();
	if (pid < 0) {
		program_fail("[DAEMON] Can't fork second time");
	}
	if (pid > 0) {
		exit(0);
	} // close second parent and return

	if (!is_logging_enabled()) {
		fd = open("/dev/null", O_RDWR, 0); // redir basic fd to /dev/null
		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > 2) close(fd); // there should be no other fd...
		}
	}

	/*resetting File Creation Mask */
	umask(027); // mask to no more than 750 (complement of 027)
	
	return SUCCESS_DAEMON;
}

