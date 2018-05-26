#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "parse/parse.h"
#include "execute/execute.h"
#include "daemon/daemon.h"
#include "daemon/daemon_socket.h"
#include "print_results/printer.h"
#include "print_results/print_results.h"

const char *DEFAULT_FORMAT			= "txt";
const char *DEFAULT_LOG_PATH		= "/tmp/SO_project.log";
const char *DEFAULT_LOG_OPTIONS 	= "start_time,end_time,total_time,exit_code";


typedef struct Arguments {
	BOOL        print_help;
    BOOL        stop_daemon;
	const char  *log_file_path;
	const char  *log_options;
	const char  *command;
    const char 	*format;
} Arguments;

Arguments * parse_arguments(int argc, char **argv);
const char * concat_strings_with_spaces (char **strings, int count);
void print_help();



int main (int argc, char *argv[]) {

	// Parse tool arguments
	Arguments *arguments = parse_arguments(argc, argv);
	if (arguments->print_help) {
		print_help();
		exit(0);
	}

    if (arguments->stop_daemon) {
        stop_daemon();
        exit(0);
    } else {
		start_daemon();
	}

	if (arguments->command == NULL || strlen(arguments->command) <= 0) {
		program_fail("No command specified. Use --help to see usage.\n");
	}


	// Parse command
    initialize_parser();
	Node *command_tree = create_tree_from_string(arguments->command);
    print_log("[RUN] Command successfully parsed\n");

	// Execute the command
    execute(command_tree);


    int log_fd = book_and_obtain_log_fd(arguments->log_file_path);
	print_results(command_tree, log_fd, arguments->format, arguments->command,
				  arguments->log_options);

	return command_tree->result->exit_code;
}

Arguments * parse_arguments(int argc, char **argv) {
	Arguments *arguments 		= malloc(sizeof(Arguments));
    arguments->print_help 		= FALSE;
    arguments->stop_daemon 		= FALSE;
	arguments->log_file_path 	= DEFAULT_LOG_PATH;
	arguments->log_options 		= DEFAULT_LOG_OPTIONS;
	arguments->format			= DEFAULT_FORMAT;
	arguments->command			= NULL;


	int i;
	for (i = 1; i < argc; i++) {
		BOOL next_argument_exists = (i + 1) < argc;

		if (strcmp(argv[i], "--help") == 0) {
			arguments->print_help = TRUE;

		} else if (strcmp(argv[i], "--log_file") == 0 && next_argument_exists) {
			// TODO: If abs, do not chdir into tool wd
			arguments->log_file_path = argv[++i];

		} else if (strcmp(argv[i], "--options") == 0 && next_argument_exists) {
			arguments->log_options = argv[++i];

		} else if (strcmp(argv[i], "--format") == 0 && next_argument_exists) {
			arguments->format = argv[++i];

		} else if (strcmp(argv[i], "--verbose") == 0) {
			enable_logging();

		} else if (strcmp(argv[i], "--stop_daemon") == 0) {
			arguments->stop_daemon = TRUE;

		} else {
			arguments->command = concat_strings_with_spaces(&argv[i], argc - i);
			break;
		}
	}

	return arguments;
}

void print_help () {
	printf("command_stats\n\n");
	printf("Usage:\n");
	printf("command_stats <options> <command>\n");
	printf("\nOptions:\n");
	printf("\t--help\t\tPrint this message;\n");
	printf("\t--log_file\tSpecify log file path;\n");
	printf("\t--format\tChoose output format (TXT or CSV);\n");
	printf("\t--options\tChoose what to include in the log file;\n");
    printf("\t--verbose\tEnable logging of the tool;\n");
    printf("\t--stop_daemon\tStops the writer daemon if running;\n");
}


const char * concat_strings_with_spaces (char **strings, int count) {
	int i;
	size_t size = (size_t) count; // spaces + null terminator
	for (i = 0; i < count; i++) {
		size += strlen(strings[i]);
	}

	char *concatenated = malloc(size);
	concatenated[0] = '\0';
	for (i = 0; i < count; i++) {
		strcat(concatenated, strings[i]);
		if (i < (count - 1)) {
			strcat(concatenated, " ");
		}
	}
	return concatenated;
}