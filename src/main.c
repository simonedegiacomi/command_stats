#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "parse.h"
#include "execute.h"
#include "wire.h"

const char *DEFAULT_LOG_PATH		= "./log.txt";
const char *DEFAULT_LOG_OPTIONS 	= "./log.txt";


typedef struct Preferences {
	BOOL print_help;
	const char *log_file_path;
	const char *log_options;
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

	// TODO: Controlla se può scrivere il file di log, e lo crea se non esiste
	//TODO: Controllo se il logger è in esecuzione, altrimenti lo avvio

	const char *command = argv[argc - 1];

	// Parse command
	initialize_parser();
	Node *command_tree = create_tree_from_string(command);

	// Connect pipe and stream together
    wire(command_tree);

	// Execute the command
    execute(command_tree);

	// TODO: Invio statistiche al logger

	return command_tree->result->exit_code;
}

Preferences * parse_preferences(int argc, char *argv[]) {
	Preferences *preferences 		= malloc(sizeof(Preferences));
	preferences->print_help 		= FALSE;
	preferences->log_file_path 		= DEFAULT_LOG_PATH;
	preferences->log_options 		= DEFAULT_LOG_OPTIONS;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			preferences->print_help 	= TRUE;
		} else if (strcmp(argv[i], "--log_file") == 0 && (i + 1) < argc) {
			preferences->log_file_path 	= argv[++i];
		} else if (strcmp(argv[i], "--options") == 0 && (i + 1) < argc) {
			preferences->log_options 	= argv[++i];
		} else if (strcmp(argv[i], "--format") == 0 && (i + 1) < argc) {

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
}