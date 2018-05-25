#ifndef COLLECT_RESULT_H
#define COLLECT_RESULT_H

#include <stdio.h>
#include "../common/common.h"
#include "../structs/node.h"

typedef enum FileFormat {
	TXT,
	CSV
} FileFormat;

FileFormat format_from_string (const char* format_string);

void print_results(Node *root, int stream_fd, FileFormat format,
				   const char *command, const char *options_string);

#endif