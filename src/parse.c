#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "parse.h"

// holds the informations of a string splitted by the operators regex
typedef struct SplitResult {
	int count;
	const char **values;
} SplitResult;
SplitResult* split_string_for_operator (const char *string, regex_t *separator);

// entry for the array that describes the priority of the operands.
typedef struct PriorityMapEntry {

	// regex to find the separator
	char 		*separator_regex;

	// compiled regex (at runtime)
	regex_t* compiled_separator;

	// function to construct a node from the splitted string
	Node *(*handler) (SplitResult*);
} PriorityMapEntry;


// constructors of the nodes from the splitted strings
Node * create_pipe_from_strings			(SplitResult *pieces);
Node * create_and_from_strings			(SplitResult *pieces);
Node * create_or_from_strings			(SplitResult *pieces);
Node * create_executable_from_string	(const char *string);

PriorityMapEntry priority_map[] = {
	{
		.separator_regex 	= "&&",
		.handler 			= create_and_from_strings
	}, {
		.separator_regex	= "\\|\\|",
		.handler 			= create_or_from_strings
	}, { 
		.separator_regex	= "[^|]\\|[^|]",
		.handler 			= create_pipe_from_strings
	}
};
const int operators_count = sizeof(priority_map) / sizeof(PriorityMapEntry);
BOOL priority_map_initialized = FALSE;

regex_t * compile_regex (const char *regex) {
	regex_t *compiled = malloc(sizeof(regex_t));
	int compile_res = regcomp(compiled, regex, REG_EXTENDED);
	if (compile_res != 0) {
		// TODO: Handle this error
		fprintf(stderr, "regex compilation failed\n");
		exit(-1);
	}
	return compiled;
}

void initialize_priority_map () {
	int i;
	for (i = 0; i < operators_count; i++) {
		priority_map[i].compiled_separator = compile_regex(priority_map[i].separator_regex);
	}
	priority_map_initialized = TRUE;
}

const char * remove_parentesis_if_alone (const char *str) {
	// TODO: Compile only the first time
	regex_t *regex = compile_regex("^[ ]*\\((.*)\\)[ ]*$");

	regmatch_t matches[2];
	int res = regexec(regex, str, 2, matches, 0);
	regmatch_t match = matches[1];

	if (res != REG_NOMATCH) {
		const char *start = str + match.rm_so;
		int to_copy = match.rm_eo - match.rm_so;
		return strndup(start, to_copy);
	}

	return strdup(str);
}

Node * create_tree_from_string (const char *command) {
	if (!priority_map_initialized) {
		initialize_priority_map();
	}

	const char* string = remove_parentesis_if_alone(command);


	int i;
	for (i = 0; i < operators_count; i++) {
		PriorityMapEntry *entry = &priority_map[i];
		SplitResult *pieces = split_string_for_operator(string, entry->compiled_separator);

		if (pieces != NULL) {
			return entry->handler(pieces);
		}
	}

	return create_executable_from_string(string);
}


int count_occurrences_of_regex (const char *string, regex_t *separator) {
	regmatch_t match;
	int count = 0;
	const char *str = string;
	while (1) {
		int res = regexec(separator, str, 1, &match, 0);

		if (res == REG_NOMATCH) {
			break;
		} else {
			count++;
			str += match.rm_eo;
		}
	};

	return count;
}

const char * obfuscate_parentesis_in_string (const char *str) {
	int len = strlen(str);
	char *obs = malloc(len * sizeof(char));

	int i;
	int parentesis = 0;
	for (i = 0; i < len; i++) {
		if (str[i] == '(') {
			parentesis++;
		}
		obs[i] = (parentesis > 0) ? '#' : str[i];
		if (str[i] == ')') {
			parentesis--;
		}
	}
	obs[len] = '\0';

	return obs;
}

SplitResult *split_string_for_operator (const char *string, regex_t *separator) {
	const char *obfuscated_string = obfuscate_parentesis_in_string(string);

	int operators_count = count_occurrences_of_regex(obfuscated_string, separator);
	if (operators_count <= 0) {
		free((void *) obfuscated_string);
		return NULL;
	}

	int pieces = operators_count + 1;
	SplitResult *split = malloc(sizeof(SplitResult));
	split->count = pieces;
	split->values = malloc(pieces * sizeof(char*));

	
	int i;
	const char *obs_str = obfuscated_string;
	const char *str = string;
	regmatch_t match;
	for (i = 0; i < pieces; i++) {
		int res = regexec(separator, obs_str, 1, &match, 0);

		const char *start = str;
		int to_copy;

		if (res == REG_NOMATCH) {
			to_copy = strlen(obs_str);
		} else {
			to_copy = match.rm_so;
		}

		split->values[i] = strndup(start, to_copy);

		obs_str += match.rm_eo;
		str += match.rm_eo;
	}

	free((void *) obfuscated_string);
	return split;
}


Node * create_executable_from_string (const char *string) {
	Node *node = malloc(sizeof(Node));
	node->type = ExecutableNode_T;
	ExecutableNode *executable = &node->value.executable;

	// Skip space
	while (string[0] == ' ') {
		string++;
	}

	char *copyToFree = strdup(string);
	char *string_copy = copyToFree;

	// Parse path
	char *path = strsep(&string_copy, " ");
	executable->path = strdup(path);

	// Count arguments
	int i;
	int arguments = 0;
	BOOL lastSpace = FALSE;
	for (i = 0; string[i] != '\0'; i++) {
		if (string[i] == ' ') {
			lastSpace = TRUE;
			arguments++;
			// Skip space
			while (string[i] == ' ') {
				i++;
			}
		} else {
			lastSpace = FALSE;
		}
	}
	if (lastSpace == FALSE) {
		arguments++;
	}
	executable->argc = arguments;

	// Parse arguments
	executable->argv = malloc(arguments * sizeof(char*));
	executable->argv[0] = strdup(executable->path);
	if (arguments == 1) {
		return node;
	}

	for (i = 1; i < arguments; i++) {
		char *arg = strsep(&string_copy, " ");
		executable->argv[i] = strdup(arg);
	}
	
	free(copyToFree);
	return node;
}


Node * create_pipe_from_strings (SplitResult *pieces) {
	Node *node = malloc(sizeof(Node));
	node->type = PipeNode_T;
	PipeNode *pipe = &node->value.pipe;

	pipe->from = create_tree_from_string(pieces->values[0]);
	if (pieces->count == 2) {
		pipe->to = create_tree_from_string(pieces->values[1]);
	} else {
		SplitResult subSplit;
		subSplit.count = pieces->count - 1;
		subSplit.values = malloc(subSplit.count * sizeof(char*));
		int i;
		for (i = 0; i < subSplit.count; i++) {
			subSplit.values[i] = pieces->values[i + 1];
		}
		pipe->to = create_pipe_from_strings(&subSplit);
	}

	return node;
}

Node * create_operands_from_string (SplitResult *pieces, NodeType type) {
	Node *node = malloc(sizeof(Node));
	node->type = type;
	OperandsNode *andNode = &node->value.operands;
	andNode->operands = pieces->count;
	andNode->nodes = malloc(pieces->count * sizeof(Node*));

	int i = 0;
	for (i = 0; i < andNode->operands; i++) {
		andNode->nodes[i] = create_tree_from_string(pieces->values[i]);
	}
	
	return node;
}

Node * create_and_from_strings (SplitResult *pieces) {
	return create_operands_from_string(pieces, AndNode_T);
}


Node * create_or_from_strings (SplitResult *pieces) {
	return create_operands_from_string(pieces, OrNode_T);
}
