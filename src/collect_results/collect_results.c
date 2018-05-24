#include <string.h>
#include <unistd.h>
#include "collect_results.h"

#include "../common/my_regex.h"



// TODO: Possiamo rimuovere queste variabili globali?
BOOL set_csv_header = FALSE;
long HASH;
const char *possible_options[] = {"exit_code", "user_cpu_time", "system_cpu_time", "clock_time", "maximum_resident_set_size"};


SplitResult * parse_options(const char *options_string) {
	const regex_t * compiled = compile_regex(",");
	SplitResult *split = split_string(options_string, compiled);
	regfree((regex_t *) compiled);
	return split;
}

long hash(const char *str) {
    long hash = 5381;
    int c;

    while ((c = *str++))
        hash = (((hash << 5) + hash) + c); /* hash * 33 + c */

    return (hash + (int) getpid()) % 100000;
}



void print_executable_node_in_txt(Node *node, FILE *stream_out, const char *command, int path_id, SplitResult *options) {

	ExecutionResult *result = node->result;
	fprintf(stream_out, "------------------------------------\n");
	fprintf(stream_out, "COMMAND\t%s\n", command);
	fprintf(stream_out, "PATH\t%s\n", node->value.executable.path);
	fprintf(stream_out, "ID #%ld.%d", HASH, path_id);
	fprintf(stream_out, "\n\n");

	for (int i = 0; i < options->count; i++) {
		const char *option = options->sub_strings[i];
		if (!strcmp(option, "exit_code") || options == NULL) {
			fprintf(stream_out, "Exit Code: %d\n", result->exit_code);
		}
		if (!strcmp(option, "user_cpu_time") || options == NULL) {
			fprintf(stream_out, "User CPU time: %ld.%06ld\n", result->user_cpu_time_used.tv_sec,
					(long) result->user_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "system_cpu_time") || options == NULL) {
			fprintf(stream_out, "System CPU time: %ld.%06ld\n", result->system_cpu_time_used.tv_sec,
					(long) result->system_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "clock_time") || options == NULL) {
			fprintf(stream_out, "Clock time: %ld, ", result->clock_time);
		}
		if (!strcmp(option, "maximum_resident_set_size") || options == NULL) {
			fprintf(stream_out, "Maximum resident set size: %ld\n", result->maximum_resident_set_size);
		}
	}
	fprintf(stream_out, "------------------------------------\n");
}


void print_executable_node_in_csv(Node *node, FILE *stream_out, const char *command, int path_id, SplitResult *options) {
	if (!set_csv_header) {
		if (options == NULL) {
			fprintf(stream_out, "#ID, COMMAND, PATH, exit code, user CPU time, system CPU time, clock time, maximum resident set size\n");
		} else {
			fprintf(stream_out, "#ID, COMMAND, PATH, ");
		}
		
		for (int i = 0; i < options->count; i++) {
			fprintf(stream_out, "%s, ", options->sub_strings[i]);
		}
		fprintf(stream_out, "\n");
		set_csv_header = TRUE;
	}

	ExecutionResult *result = node->result;
	fprintf(stream_out, "#%ld.%d, ", HASH, path_id);
	fprintf(stream_out, "%s, ", command);
	fprintf(stream_out, "%s, ", node->value.executable.path);

	for (int i = 0; i < options->count; i++) {
		const char *option = options->sub_strings[i];
		if (!strcmp(option, "exit_code") || options == NULL) {
			fprintf(stream_out, "%d, ", result->exit_code);
		}
		if (!strcmp(option, "user_cpu_time") || options == NULL) {
			fprintf(stream_out, "%ld.%06ld, ", result->user_cpu_time_used.tv_sec, (long) result->user_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "system_cpu_time") || options == NULL) {
			fprintf(stream_out, "%ld.%06ld, ", result->system_cpu_time_used.tv_sec,
					(long) result->system_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "clock_time") || options == NULL) {
			fprintf(stream_out, "%ld, ", result->clock_time);
		}
		if (!strcmp(option, "maximum_resident_set_size") || options == NULL) {
			fprintf(stream_out, "%ld, ", result->maximum_resident_set_size);
		}
	}

}


void print_executable_node(Node *node, FILE *stream_out, FileFormat format, const char *command, int path_id, SplitResult *options) {
	switch (format) {
		case TXT:
			print_executable_node_in_txt(node, stream_out, command, path_id, options);
			break;
		case CSV:
			print_executable_node_in_csv(node, stream_out, command, path_id, options);
			break;
	}
}


void collect_and_print_results_rec(Node *node, FILE *stream_out, FileFormat format, const char *command, int path_id, SplitResult *options) {
	OperandsNode *operandNode;
	switch (node->type) {
		case PipeNode_T:
		case AndNode_T:
		case OrNode_T:
		case SemicolonNode_T:
			operandNode = &(node->value.operands);
			for (int i = 0; i < operandNode->count; i++) {
				collect_and_print_results_rec(operandNode->nodes[i],stream_out,format,command,path_id,options);
			}
			break;
		case ExecutableNode_T:
			print_executable_node(node,stream_out,format,command,path_id+1,options);
			break;
	}
	
}


void collect_and_print_results(Node *node, int out_fd, FileFormat format, const char *command, const char *options_string) {
	FILE *stream_out = fdopen(out_fd, "w");

	HASH = hash(command);
	SplitResult *options = parse_options(options_string);
	collect_and_print_results_rec(node, stream_out, format, command, 0, options);

	fclose(stream_out);
}
