#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "parse/parse.h"
#include "execute/execute.h"
#include "daemon/daemon.h"
#include "daemon/daemon_socket.h"
#include "print_results/print_results.h"

const char *DEFAULT_FORMAT			= "txt";
const char *DEFAULT_LOG_PATH		= "/tmp/SO_project.log";
const char *DEFAULT_LOG_OPTIONS 	= "invocation_failed,pid,start_time,end_time,total_time,user_cpu_time,system_cpu_time,exit_code";


typedef struct Arguments {
	BOOL        print_help;
    BOOL        stop_daemon;
	const char  *log_file_path;
	const char  *log_options;
	const char  *command;
    const char 	*format;
} Arguments;

Arguments * 	parse_arguments				(int argc, char **argv);
const char * 	concat_strings_with_spaces	(char **strings, int count);
void 			print_help					();



int main (int argc, char *argv[]) {

	// Parse tool arguments
	Arguments *arguments = parse_arguments(argc, argv);
	if (arguments->print_help) {
		print_help();
		exit(0);
	}

    if (arguments->stop_daemon) {
        stop_daemon();
		printf("Daemon stopped.\n");
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
	finalize_parser();
    print_log("[RUN] Command successfully parsed\n");

	// Execute the command
    execute(command_tree);
    print_log("[EXECUTE] Execution terminated\n");

	// Request pipe from daemon
    int log_fd = book_and_obtain_log_fd(arguments->log_file_path);

    // Print logs
	print_results(command_tree, log_fd, arguments->format, arguments->command,
				  arguments->log_options);

    // Exit with the same exit code as the root command
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

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			arguments->print_help = TRUE;

		} else if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log-file") == 0) && next_argument_exists) {
			arguments->log_file_path = argv[++i];

		} else if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--options") == 0) && next_argument_exists) {
			arguments->log_options = argv[++i];

		} else if ((strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) && next_argument_exists) {
			arguments->format = argv[++i];

		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			enable_logging();

		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--stop-daemon") == 0) {
			arguments->stop_daemon = TRUE;

		} else {
			arguments->command = concat_strings_with_spaces(&argv[i], argc - i);
			break;
		}
	}

	return arguments;
}

void print_help () {
	printf("\n    command_stats - log execution statistics of shell commands\n\n");
	
	printf("Usage:\n");
	printf("    command_stats [options] [command]\n");
	
	printf("\nOptions:\n");
	printf("    -h, --help         Print this message;\n");
	printf("    -l, --log-file     Specify log file path;\n");
	printf("    -f, --format       Choose output format (TXT, CSV or HTML);\n");
	printf("    -o, --options      Choose what to include in the log file, see \"Arguments\n");
	printf("                       for --options\" section for available options;\n");
    printf("    -v, --verbose      Enable logging of the tool;\n");
    printf("    -s, --stop-daemon  Stop the writer daemon if running;\n");
	
	printf("\nArguments for --options:\n");
	printf("    Specify the options needed separated by commas, with no spaces:\n");
	printf("        * pid:                Process ID;\n");
	printf("        * invocation_failed:  True if the executable couldn't start;\n");
	printf("        * exit_code:          Exit code;\n");
	printf("        * start_time:         Clock starting time;\n");
	printf("        * end_time:           Clock ending time of the execution;\n");
	printf("        * total_time:         Total clock time that the executable was running;\n");
	printf("        * user_cpu_time:      Total cpu time effectively used in user mode;\n");
	printf("        * system_cpu_time:    Total cpu time effectively used in kernel mode;\n");
	printf("        * maximum_resident_set_size:\n");
	printf("                              Memory of the executable that was in RAM;\n\n");
}

/**
 * Utility to concatenate an array of string into a single string. Each string is
 * separated by a space.
 */
const char * concat_strings_with_spaces (char **strings, int count) {
	int i;
	size_t size = (size_t) count; // n spaces + null terminator
	for (i = 0; i < count; i++) {
		size += strlen(strings[i]); // length of each string
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
