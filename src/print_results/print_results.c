#include <string.h>
#include <unistd.h>
#include "collect_results.h"

#include "../common/my_regex.h"

typedef struct PrinterContext {
    const char      *command;
    long            index;
    int             command_subindex;
    SplitResult     *attributes;
    FILE            *out;
    FileFormat      format;
} PrinterContext;

typedef void (*ToString)(PrinterContext*, Node*);

typedef struct Option {
    const char *keyword;
    ToString to_string;
} Option;


void pid_to_string (PrinterContext *context, Node* node) {
    fprintf(context->out, "wow %d\n", node->pid);
}


long hash(const char *str) {
    long hash = 5381;
    int c;

    while ((c = *str++))
        hash = (((hash << 5) + hash) + c); /* hash * 33 + c */

    return (hash + (int) getpid()) % 100000;
}


Option options[] = {
    {
        .keyword = "pid",
        .to_string = pid_to_string
    }/*,
    {
        .keyword = "exit_code",
        .to_string = exit_code_to_string
    },
    {
        .keyword = "execution_failed",
        .to_string = execution_failed_to_string
    },
    {
        .keyword = "start_time",
        .to_string = start_time_to_string
    },
    {
        .keyword = "end_time",
        .to_string = end_time_to_string
    },
    {
        .keyword = "total_time",
        .to_string = total_time_to_string
    },
    {
        .keyword = "user_cpu_time",
        .to_string = user_cpu_time_to_string
    },
    {
        .keyword = "system_cpu_time",
        .to_string = system_cpu_time_to_string
    },
    {
        .keyword = "maximum_resident_set_size",
        .to_string = maximum_resident_set_size_to_string
    },*/
};
int options_count = sizeof(options) / sizeof(Option);


FileFormat format_from_string (const char* format_string) {
    if (strcmp(format_string, "CSV") == 0) {
        return CSV;
    } else {
        return TXT;
    }
}

SplitResult * parse_attributes(const char *options_string);
void print_executable_node(Node *node, PrinterContext *context);
void print_results_rec(Node *node, PrinterContext *context);
void print_executable_node_in_txt(Node *node, PrinterContext *context);
void print_executable_node_in_csv(Node *node, PrinterContext *context);

void print_results(Node *root, int out_fd, FileFormat format,
                   const char *command, const char *options_string) {

    PrinterContext context = {
        .command            = command,
        .index              = hash(command),
        .command_subindex   = 1,
        .format             = format,
        .attributes         = parse_attributes(options_string)
    };

    context.out = fdopen(out_fd, "w");
    print_results_rec(root, &context);
    fclose(context.out);
}



SplitResult * parse_attributes(const char *options_string) {

    const regex_t * compiled = compile_regex(",");
    SplitResult *split = split_string(options_string, compiled);

    regfree((regex_t *) compiled);
    return split;
}


void print_results_rec(Node *node, PrinterContext *context) {

    if (is_operand_node(node)) {
        OperandsNode *operandNode;
        operandNode = &(node->value.operands);
        int i;
        for (i = 0; i < operandNode->count; i++) {
            print_results_rec(operandNode->nodes[i], context);
        }
    } else {
        print_executable_node(node, context);
    }
}


void print_executable_node(Node *node, PrinterContext *context) {
    switch (context->format) {
        case TXT:
            print_executable_node_in_txt(node, context);
            break;

        case CSV:
            print_executable_node_in_csv(node, context);
            break;
    }
}



ToString find_to_string (const char *keyword) {
    int i;
    for (i = 0; i < options_count; i++) {
        if (strcmp(options[i].keyword, keyword) == 0) {
            return options[i].to_string;
        }
    }


    return NULL;
}


void print_executable_node_in_txt(Node *node, PrinterContext *context) {

    ExecutionResult *result = node->result;
    fprintf(context->out, "------------------------------------\n");
    fprintf(context->out, "COMMAND\t%s\n", context->command);
    fprintf(context->out, "PATH\t%s\n", node->value.executable.path);
    fprintf(context->out, "ID #%ld.%d", context->index, context->command_subindex++);
    fprintf(context->out, "\n\n");




    int i;
    for (i = 0; i < context->attributes->count; i++) {
        const char *option = context->attributes->sub_strings[i];
        find_to_string(option)(context, node);
    }


/*
    if (attributes->pid) {
        fprintf(context->out, "PID: %d\n", node->pid);
    }
    if (attributes->exit_code) {
        fprintf(context->out, "Exit Code: %d\n", result->exit_code);
    }
    if (attributes->execution_failed) {
        fprintf(context->out, "Execution OK: %d\n", !result->execution_failed);
    }
    if (attributes->start_time) {
        fprintf(context->out, "Start time: %ld\n", result->start_time);
    }
    if (attributes->end_time) {
        fprintf(context->out, "End time: %ld\n", result->end_time);
    }
    if (attributes->total_time) {
        fprintf(context->out, "Total time: %ld\n",
                get_total_time(result->start_time, result->end_time));
    }
    if (attributes->user_cpu_time) {
        fprintf(context->out, "User CPU time: %ld.%06ld\n",
                result->user_cpu_time_used.tv_sec,
                (long) result->user_cpu_time_used.tv_usec);
    }
    if (attributes->system_cpu_time) {
        fprintf(context->out, "System CPU time: %ld.%06ld\n",
                result->system_cpu_time_used.tv_sec,
                (long) result->system_cpu_time_used.tv_usec);
    }
    if (attributes->maximum_resident_set_size) {
        fprintf(context->out, "Maximum resident set size: %ld\n",
                result->maximum_resident_set_size);
    }
*/

    fprintf(context->out, "------------------------------------\n");
}






void print_executable_node_in_csv(Node *node, PrinterContext *context) {
	/*if (!set_csv_header) {
		if (options == NULL) {
			fprintf(stream_out, "#ID, COMMAND, PATH, exit code, user CPU time, system CPU time, clock time, maximum resident set size\n");
		} else {
			fprintf(stream_out, "#ID, COMMAND, PATH, ");
			int i;
			for (i = 0; i < options->count; i++) {
				if (check_option_is_valid(options->sub_strings[i])) {
					fprintf(context->out, "%s, ", options->sub_strings[i]);
				}
			}
			fprintf(context->out, "\n");
		}
		
		int i;
		for (i = 0; i < options->count; i++) {
			fprintf(context->out, "%s, ", options->sub_strings[i]);
		}
		fprintf(context->out, "\n");
		set_csv_header = TRUE;
	}

	ExecutionResult *result = node->result;
	fprintf(context->out, "#%ld.%d, ", HASH, path_id);
	fprintf(context->out, "%s, ", command);
	fprintf(context->out, "%s, ", node->value.executable.path);

	int i;
	for (i = 0; i < options->count; i++) {
		const char *option = options->sub_strings[i];
		if (!strcmp(option, "exit_code") || options == NULL) {
			fprintf(context->out, "%d\n, ", result->exit_code);
		}
		if (!strcmp(option, "execution_failed") || options == NULL) {
			fprintf(context->out, "%d\n, ", !result->execution_failed);
		}
		if (!strcmp(option, "start_time") || options == NULL) {
			fprintf(context->out, "%ld\n, ", result->start_time);
		}
		if (!strcmp(option, "end_time") || options == NULL) {
			fprintf(context->out, "%ld\n, ", result->end_time);
		}
		if (!strcmp(option, "total_time") || options == NULL) {
			fprintf(context->out, "%ld\n, ", get_total_time(result->start_time,result->end_time));
		}
		if (!strcmp(option, "user_cpu_time") || options == NULL) {
			fprintf(context->out, "%ld.%06ld\n, ", result->user_cpu_time_used.tv_sec,
					(long) result->user_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "system_cpu_time") || options == NULL) {
			fprintf(context->out, "%ld.%06ld\n, ", result->system_cpu_time_used.tv_sec,
					(long) result->system_cpu_time_used.tv_usec);
		}
		if (!strcmp(option, "maximum_resident_set_size") || options == NULL) {
			fprintf(context->out, "%ld\n, ", result->maximum_resident_set_size);
		}
		if (!strcmp(option, "pid") || options == NULL) {
			fprintf(context->out, "%d\n, ", node->pid);
		}
	}
*/
}






