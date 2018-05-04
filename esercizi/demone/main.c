#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void die (const char *str) {
	printf("%s\n", str);
	exit(-1);
}

void daemonise() {
    // Fork, allowing the parent process to terminate.
    pid_t pid = fork();
    if (pid == -1) {
        die("failed to fork while daemonising (errno=)");
    } else if (pid != 0) {
        _exit(0);
    }

    // Start a new session for the daemon.
    /*if (setsid()==-1) {
        die("failed to become a session leader while daemonising(errno=)");
    }*/

    // Fork again, allowing the parent process to terminate.
    //signal(SIGHUP,SIG_IGN);
    pid=fork();
    if (pid == -1) {
        die("failed to fork while daemonising (errno=)");
    } else if (pid != 0) {
        _exit(0);
    }

    // Set the current working directory to the root directory.
    if (chdir("/") == -1) {
        die("failed to change working directory while daemonising (errno=)");
    }

    // Set the user file creation mask to zero.
    umask(0);

    // Close then reopen standard file descriptors.
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    if (open("/dev/null",O_RDONLY) == -1) {
        die("failed to reopen stdin while daemonising (errno=)");
    }
    if (open("/dev/null",O_WRONLY) == -1) {
        die("failed to reopen stdout while daemonising (errno=)");
    }
    if (open("/dev/null",O_RDWR) == -1) {
        die("failed to reopen stderr while daemonising (errno=)");
    }
}

const char *fileName = "/tmp/demone";

void on_signal (int signal) {
    if (signal == )
}

int main (int argc, char *argv[]) {

    signal();
    struct stat state;
    int res = stat(fileName, &state);
    if (res == 0) {
        printf("Il demone è già in esecuzione\n");
        exit(0);
    } else {
        open(fileName, O_CREAT);

    	daemonise();
    	while (1) {
    		printf("Demone\n");
    		sleep(1);
    	}
    }
}
