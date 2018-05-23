#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "parse.h"
#include "execute.h"
#include "wire.h"
#include "collect_results.h"

const char *DEFAULT_LOG_PATH		= "./log.txt";
const char *DEFAULT_LOG_OPTIONS 	= "./log.txt";


typedef struct Preferences {
	BOOL print_help;
	const char *log_file_path;
	const char *log_options;
    FileFormat format;
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

	//TODO: Controlla se può scrivere il file di log, e lo crea se non esiste
	//TODO: Controllo se il logger è in esecuzione, altrimenti lo avvio


	// Parse command
	// TODO: Decidere se inizializzare esplicitamente il parser
    initialize_parser();
	const char *input = argv[argc - 1];
	Node *command_tree = create_tree_from_string(input);


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
            i++;
            if (strcmp(argv[i], "TXT") == 0) {
                preferences->format = TXT;
            } else if (strcmp(argv[i], "CSV") == 0) {
                preferences->format = CSV;
            }
		} else if (strcmp(argv[i], "--verbose") == 0) {
            enable_logging();
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
}