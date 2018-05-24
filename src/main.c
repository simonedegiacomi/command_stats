#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "parse/parse.h"
#include "execute/execute.h"
#include "collect_results/collect_results.h"
#include "daemon/daemon.h"
#include "daemon/daemon_socket.h"
#include "common/syscalls_wrappers.h"

const char *DEFAULT_LOG_PATH		= "/tmp/SO_project_log.txt";
const char *DEFAULT_LOG_OPTIONS 	= "pid";


typedef struct Preferences {
	BOOL        print_help;
    BOOL        stop_daemon;
	const char  *log_file_path;
	const char  *log_options;
    FileFormat  format;
} Preferences;

Preferences * parse_preferences(int argc, char *argv[]);
void print_help();



int main (int argc, char *argv[]) {

	// Parse tool arguments
	Preferences *preferences = parse_preferences(argc, argv);
	if (preferences->print_help) {
		print_help();
		exit(0);
	}

    start_daemon();

    if (preferences->stop_daemon) {
        //stop_daemon();
        exit(0);
    }


	// Parse command
	// TODO: Decidere se inizializzare esplicitamente il parser
    initialize_parser();
	const char *input = argv[argc - 1];
	Node *command_tree = create_tree_from_string(input);
    print_log("[RUN] Command successfully parsed\n");

	// Execute the command
    execute(command_tree);


    int log_fd = obtain_log_fd(preferences->log_file_path);
    collect_and_print_results(command_tree, log_fd, preferences->format, input, preferences->log_options);

	return command_tree->result->exit_code;
}

Preferences * parse_preferences(int argc, char *argv[]) {
	Preferences *preferences 		= malloc(sizeof(Preferences));
    preferences->print_help 		= FALSE;
    preferences->stop_daemon 		= FALSE;
	preferences->log_file_path 		= DEFAULT_LOG_PATH;
	preferences->log_options 		= DEFAULT_LOG_OPTIONS;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			preferences->print_help 	= TRUE;
		} else if (strcmp(argv[i], "--log_file") == 0 && (i + 1) < argc) {

            // TODO: Transform in absolute if needed
			preferences->log_file_path 	= argv[++i];
		} else if (strcmp(argv[i], "--options") == 0 && (i + 1) < argc) {
			preferences->log_options 	= argv[++i];
		} else if (strcmp(argv[i], "--format") == 0 && (i + 1) < argc) {
            i++;
            if (strcmp(argv[i], "TXT") == 0) {
                preferences->format = TXT;
            } else if (strcmp(argv[i], "CSV") == 0) {
                preferences->format = CSV;
            }
		} else if (strcmp(argv[i], "--verbose") == 0) {
            enable_logging();
        } else if (strcmp(argv[i], "--stop_daemon") == 0) {
            preferences->stop_daemon = TRUE;
        }

	}

	return preferences;
}

void print_help () {
	printf("NAME OF TOOL\n\n");
	printf("Usage:\n");
	printf("./run <options> <command>\n");
	printf("\nOptions:\n");
	printf("\t--help\t\tPrint this message;\n");
	printf("\t--log_file\tSpecify log file path;\n");
	printf("\t--format\tChoose output format (TXT or CSV);\n");
	printf("\t--options\tChoose what to include in the log file;\n");
    printf("\t--verbose\tEnable logging of the tool;\n");
    printf("\t--stop_daemon\tStops the writer daemon if running;\n");
}