#ifndef COLLECT_RESULT_H
#define COLLECT_RESULT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "parse.h"
#include "structs.h"
#include "my_regex.h"

typedef enum FileFormat {
	TXT,
	CSV
} FileFormat;

void collect_and_print_results(Node *node, FILE *stream_out, FileFormat format, char *command, char *options_string);

//TODO: test only

const char ** parse_options(char *options_string);

#endif