#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include "structs.h"


#define BOOL char
#define TRUE 1
#define FALSE 0

// pipe file descriptor indexes
#define WRITE_INTO_PIPE 1
#define READ_FROM_PIPE 0


#endif