#ifndef COLLECT_RESULT_H
#define COLLECT_RESULT_H

#include "common.h"


typedef enum FileFormat {
	TXT,
	CSV
} FileFormat;

void collect_and_print_results(Node *node, FILE *stream_out, FileFormat format, char *command);



#endif