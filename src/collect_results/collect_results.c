#include <string.h>
#include <unistd.h>
#include "collect_results.h"

#include "../common/my_regex.h"



// TODO: Possiamo rimuovere queste variabili globali?
BOOL set_csv_header = FALSE;
long HASH;


BOOL check_option_is_valid(const char *option) {
	const char options[9][255] = {"exit_code", "execution_failed", "start_time", "end_time", "total_time", "user_cpu_time", "system_cpu_time", "maximum_resident_set_size", "pid"};
	const int number_of_options = 9;
	int i;
	for (i = 0; i < number_of_options; i++) {
		if (!strcmp(option,options[i])) {
			return TRUE;
		}
	}
	return FALSE;
}

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
		} else if (!strcmp(option, "execution_failed") || options == NULL) {
			fprintf(stream_out, "Execution OK: %d\n", !result->execution_failed);
		} else if (!strcmp(option, "start_time") || options == NULL) {
			fprintf(stream_out, "Start time: %ld\n", result->start_time);
		} else if (!strcmp(option, "end_time") || options == NULL) {
			fprintf(stream_out, "End time: %ld\n", result->end_time);
		} else if (!strcmp(option, "total_time") || options == NULL) {
			fprintf(stream_out, "Total time: %ld\n", get_total_time(result->start_time,result->end_time));
		} else if (!strcmp(option, "user_cpu_time") || options == NULL) {
			fprintf(stream_out, "User CPU time: %ld.%06ld\n", result->user_cpu_time_used.tv_sec,
					(long) result->user_cpu_time_used.tv_usec);
		} else if (!strcmp(option, "system_cpu_time") || options == NULL) {
			fprintf(stream_out, "System CPU time: %ld.%06ld\n", result->system_cpu_time_used.tv_sec,
					(long) result->system_cpu_time_used.tv_usec);
		} else if (!strcmp(option, "maximum_resident_set_size") || options == NULL) {
			fprintf(stream_out, "Maximum resident set size: %ld\n", result->maximum_resident_set_size);
		} else if (!strcmp(option, "pid") || options == NULL) {
			fprintf(stream_out, "PID: %d\n", node->pid);
		} else {
			fprintf(stream_out, "%s is not a valid type of data.\n", option);
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
			int i;
			for (i = 0; i < options->count; i++) {
				if (check_option_is_valid(options->sub_strings[i])) {
					fprintf(stream_out, "%s, ", options->sub_strings[i]);
				}
			}
			fprintf(stream_out, "\n");
		}
		set_csv_header = TRUE;
	}

	ExecutionResult *result = node->result;
	fprintf(stream_out, "#%ld.%d, ", HASH, path_id);
	fprintf(stream_out, "%s, ", command);
	fprintf(stream_out, "%s, ", node->value.executable.path);

	for (int i = 0; i < options->count; i++) {
		const char *option = options->sub_strings[i];
		if (!strcmp(option, "exit_code") || options == NULL) {
			fprintf(stream_out, "%d\n, ", result->exit_code);
		}
		if (!strcmp(option, "execution_failed") || options == NULL) {
			fprintf(stream_out, "%d\n, ", !result->execution_failed);
		}
		if (!strcmp(option, "start_time") || options == NULL) {
			fprintf(stream_out, "%ld\n, ", result->start_time);
		}
		if (!strcmp(option, "end_time") || options == NULL) {
			fprintf(stream_out, "%ld\n, ", result->end_time);
		}
		if (!strcmp(option, "total_time") || options == NULL) {
			fprintf(stream_out, "%ld\n, ", get_total_time(result->start_time,result->end_time));
		}
		if (!strcmp(option, "user_cpu_time") || options == NULL) {
			fprintf(stream_out, "%ld.%06ld\n, ", result->user_cpu_time_used.tv_sec,
					(long) result->user_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "system_cpu_time") || options == NULL) {
			fprintf(stream_out, "%ld.%06ld\n, ", result->system_cpu_time_used.tv_sec,
					(long) result->system_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "maximum_resident_set_size") || options == NULL) {
			fprintf(stream_out, "%ld\n, ", result->maximum_resident_set_size);
		}
		if (!strcmp(option, "pid") || options == NULL) {
			fprintf(stream_out, "%d\n, ", node->pid);
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
