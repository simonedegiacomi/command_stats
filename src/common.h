#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>


#define BOOL char
#define TRUE 1
#define FALSE 0

// pipe file descriptor indexes
#define WRITE_INTO_PIPE 1
#define READ_FROM_PIPE 0


#endif