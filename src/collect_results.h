#ifndef COLLECT_RESULT_H
#define COLLECT_RESULT_H

#include <stdio.h>
#include "common.h"
#include "structs.h"

typedef enum FileFormat {
	TXT,
	CSV
} FileFormat;


void collect_and_print_results(Node *node, FILE *stream_out, FileFormat format, char *command, char *options_string);



#endif