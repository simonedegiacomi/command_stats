#include "collect_results.h"

BOOL set_csv_header = FALSE;
long HASH;


long hash(char *str) {
    long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (hash + (int) getpid()) % 100000;
}

void print_executable_node_in_txt(Node *node, FILE *stream_out, char *command, int path_id) {

	ExecutionResult *result = node->result;
	fprintf(stream_out, "------------------------------------\n");
	fprintf(stream_out, "COMMAND\t%s\n", command);
	fprintf(stream_out, "PATH\t%s\n", node->value.executable.path);
	fprintf(stream_out, "ID #%ld.%d", HASH, path_id);
	fprintf(stream_out, "\n\n");
	fprintf(stream_out, "Exit Code: %d\n", result->exit_code);
	fprintf(stream_out, "User CPU time: %ld.%06ld\n", result->user_cpu_time_used.tv_sec, result->user_cpu_time_used.tv_usec);
	fprintf(stream_out, "System CPU time: %ld.%06ld\n", result->system_cpu_time_used.tv_sec, result->system_cpu_time_used.tv_usec);
	fprintf(stream_out, "Maximum resident set size: %ld\n", result->maximum_resident_set_size);
	fprintf(stream_out, "------------------------------------\n");
}


void print_executable_node_in_csv(Node *node, FILE *stream_out, char *command, int path_id) {
	if (!set_csv_header) {
		fprintf(stream_out, "#ID, COMMAND, PATH, exit code, user CPU time, system CPU time, clock time, maximum resident set size\n");
		set_csv_header = TRUE;
	}
	ExecutionResult *result = node->result;
	fprintf(stream_out, "#%ld.%d, ", HASH, path_id);
	fprintf(stream_out, "%s, ", command);
	fprintf(stream_out, "%s, ", node->value.executable.path);
	fprintf(stream_out, "%d, ", result->exit_code);
	fprintf(stream_out, "%ld.%06ld, ", result->user_cpu_time_used.tv_sec, result->user_cpu_time_used.tv_usec);
	fprintf(stream_out, "%ld.%06ld, ", result->system_cpu_time_used.tv_sec, result->system_cpu_time_used.tv_usec);
	fprintf(stream_out, "%ld, ", result->clock_time);
	fprintf(stream_out, "%ld, ", result->maximum_resident_set_size);

}

void print_executable_node(Node *node, FILE *stream_out, FileFormat format, char *command, int path_id) {
	switch (format) {
		case TXT:
			print_executable_node_in_txt(node,stream_out,command,path_id);
			break;
		case CSV:
			print_executable_node_in_csv(node,stream_out,command,path_id);
			break;
	}
}


void collect_and_print_results_rec(Node *node, FILE *stream_out, FileFormat format, char *command, int path_id) {
	OperandsNode *operandNode;
	switch (node->type) {
		case PipeNode_T:
		case AndNode_T:
		case OrNode_T:
			operandNode = &(node->value.operands);
			for (int i = 0; i < operandNode->count; i++) {
				collect_and_print_results_rec(operandNode->nodes[i],stream_out,format,command,path_id);
			}
			break;
		case ExecutableNode_T:
			print_executable_node(node,stream_out,format,command,path_id+1);
			break;
	}
	
}

void collect_and_print_results(Node *node, FILE *stream_out, FileFormat format, char *command) {
	HASH = hash(command);
	collect_and_print_results_rec(node,stream_out,format,command,0);
}
