#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "common.h"
#include "parse.h"

const char * remove_brackets_if_alone(const char *str);

// keep together substrings split by the same operator
typedef struct SplitResult {
	int         count;
	const char  **sub_strings;
} SplitResult;

SplitResult * split_string_for_operator (const char *string, regex_t *separator);

// contains the substring of a command and, if any, the file names to which redirect the input/output
typedef struct RedirectSplit {
    const char  *command;
    BOOL        has_redirects;

    Stream      *in, *out;
} RedirectSplit;

RedirectSplit *new_redirect_split () {
    RedirectSplit *split = malloc(sizeof(RedirectSplit));;
    split->in = split->out = NULL;
    return split;
}


RedirectSplit * split_redirect_if_last_right(const char *str);

void redirect_streams_of_node_to_files(Node *parsed, RedirectSplit *string_and_redirects);




// entry for the array that describes the priority of the operands.
typedef struct PriorityMapEntry {

	char 		*separator_regex;

	regex_t*    compiled_separator;

	// function to construct a node from the split string
	Node *(*handler) (SplitResult*);
} PriorityMapEntry;


// constructors of the nodes from the split strings
Node * create_pipe_from_strings			(SplitResult *pieces);
Node * create_and_from_strings			(SplitResult *pieces);
Node * create_or_from_strings			(SplitResult *pieces);
Node * create_executable_from_string	(SplitResult *pieces);

PriorityMapEntry priority_map[] = {
	{
		.separator_regex 	= "&&",
		.handler 			= create_and_from_strings
	}, { // Or operator
		.separator_regex	= "\\|\\|",
		.handler 			= create_or_from_strings
	}, { // Pipe operator
		.separator_regex	= "[^|]\\|[^|]",
		.handler 			= create_pipe_from_strings
	}
};
const int operators_count = sizeof(priority_map) / sizeof(PriorityMapEntry);


BOOL parser_initialized = FALSE;

void initialize_regexes();
regex_t * compile_regex (const char *regex);



void initialize_parser() {
    if (parser_initialized) {
        return;
    }
    initialize_regexes();
	parser_initialized = TRUE;
}

void initialize_regexes() {
    // priority map regexes
    int i;
    for (i = 0; i < operators_count; i++) {
        priority_map[i].compiled_separator = compile_regex(priority_map[i].separator_regex);
    }

    // other regex

}

regex_t * compile_regex (const char *regex) {
	regex_t *compiled = malloc(sizeof(regex_t));
	int compile_res = regcomp(compiled, regex, REG_EXTENDED);
	if (compile_res != 0) {
		char error_str[1000];
		regerror(compile_res, compiled, error_str, sizeof(error_str));
		fprintf(stderr, "[PARSER] Regex compilation failed: %s\n", error_str);


		exit(-1);
	}
	return compiled;
}



Node * create_tree_from_string (const char *raw_string) {
    initialize_parser();

    RedirectSplit *string_and_redirects = split_redirect_if_last_right(raw_string);
    const char *string                   = remove_brackets_if_alone(string_and_redirects->command);

    int i;
    Node *parsed = NULL;
    for (i = 0; i < operators_count; i++) {
        PriorityMapEntry *entry = &priority_map[i];
        SplitResult *pieces     = split_string_for_operator(string, entry->compiled_separator);

        if (pieces != NULL) {
            parsed = entry->handler(pieces);
            break;
        }
    }

    if (parsed == NULL) {
        SplitResult fake_split = {
                .count = 1,
                .sub_strings = &string
        };
        parsed = create_executable_from_string(&fake_split);
    }

    if (string_and_redirects->has_redirects) {
        redirect_streams_of_node_to_files(parsed, string_and_redirects);
    }


    return parsed;
}


const char * remove_brackets_if_alone(const char *str) {
	// TODO: Compile only the first time
    char *const MATCH_STRING_INSIDE_BRACKETS_IF_ALONE = "^[ ]*\\((.*)\\)[ ]*$";
	regex_t *regex = compile_regex(MATCH_STRING_INSIDE_BRACKETS_IF_ALONE);

	regmatch_t matches[2];
	int res = regexec(regex, str, 2, matches, 0);
	regmatch_t match = matches[1];

	if (res != REG_NOMATCH) {
		const char *start = str + match.rm_so;
		size_t to_copy = (size_t) (match.rm_eo - match.rm_so);
		return strndup(start, to_copy);
	}

	return strdup(str);
}

const char *create_string_from_match(const char *str, regmatch_t *match) {
    const char *start   = str + match->rm_so;
    const char *end     = str + match->rm_eo;
    size_t to_copy      = end - start;
    return strndup(start, to_copy);
}


RedirectSplit * create_redirect_with_command_from_string (const char *str) {
    RedirectSplit *split = new_redirect_split();

    // TODO: Compile only the first time
    char *const MATCH_REDIRECT_AND_FILE_NAME_IF_LAST = "((>>|>|<) *([a-zA-Z\\.]*))+$";
    regex_t *regex = compile_regex(MATCH_REDIRECT_AND_FILE_NAME_IF_LAST); // explain groups and SPACE*

    regmatch_t match;
    int res = regexec(regex, str, 1, &match, 0);
    if (res != REG_NOMATCH) {
        const char *start = str;
        const char *end = str + match.rm_so;
        size_t to_copy = end - start;


        split->has_redirects = TRUE;
        split->command = strndup(start, to_copy);
    } else {
        split->has_redirects = FALSE;
        split->command = strdup(str);
    }

    return split;
}

void fill_redirect_with_redirects_and_file_names(RedirectSplit *split, const char *string) {
    // TODO: Compile only the first time
    char *const MATCH_REDIRECT_AND_FILE_NAME_IF_LAST = "((>>|>|<) *([a-zA-Z\\.]*))+$";
    regex_t *regex = compile_regex(MATCH_REDIRECT_AND_FILE_NAME_IF_LAST); // explain groups and SPACE*

    regmatch_t matches[4];
    for (const char *str = string; regexec(regex, str, 4, matches, 0) != REG_NOMATCH; str += matches[3].rm_eo) {
        const char *redirect_type   = create_string_from_match(str, &matches[2]);
        const char *file_name       = create_string_from_match(str, &matches[3]);

        Stream *stream;
        if (redirect_type[0] == '>') {
            if (split->out == NULL) {
                split->out = malloc(sizeof(Stream));
            }
            stream = split->out;
            stream->options.file.open_flag = O_WRONLY;

            if (strlen(redirect_type) == 2) { // >>
                stream->options.file.open_flag |= O_WRONLY;
            }
        } else {
            if (split->in == NULL) {
                split->in = malloc(sizeof(Stream));
            }
            stream = split->in;
            stream->options.file.open_flag = O_RDONLY;
        }
        stream->type                 = FileStream_T;
        stream->options.file.name    = file_name;
    }
}


RedirectSplit * split_redirect_if_last_right(const char *str) {
    RedirectSplit *split = create_redirect_with_command_from_string(str);

    if (split->has_redirects) {
        fill_redirect_with_redirects_and_file_names(split, str);
    }

    return split;
}

void redirect_streams_of_node_to_files(Node *parsed, RedirectSplit *string_and_redirects) {
    if (string_and_redirects->in != NULL) {
        parsed->stdins = wrap_stream_into_array(string_and_redirects->in);
    }
    if (string_and_redirects->out != NULL) {
        parsed->stdout = string_and_redirects->out;
    }
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
	size_t len = strlen(str);
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
	split->sub_strings = malloc(pieces * sizeof(char*));


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

		split->sub_strings[i] = strndup(start, to_copy);

		obs_str += match.rm_eo;
		str += match.rm_eo;
	}

	free((void *) obfuscated_string);
	return split;
}


Node * create_executable_from_string (SplitResult *pieces) {
    const char *string = pieces->sub_strings[0];
	Node *node = new_node();
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
	free(copyToFree);

	// WARNING: unreadable code below!
	// TODO: Use regex

	int i;
	int count = 0;
	for (i = 0; string[i] != '\0'; i++) {
		// Skip space
		while (string[i] == ' ') {
			i++;
		}

		if (string[i] == '\"') {
			i++;
			while (string[i] != '\0' && (string[i] != '\"' || string[i - 1] == '\\')) {
				i++;
			}
			if (string[i] != '\0' && string[i + 1] != '\"') {
				count++;
			}
		} else if (string[i] != '\0') {
			i++;
			while (string[i] != '\0' && (string[i] != ' ' || string [i - 1] == '\\')) {
				i++;
			}
			count++;
			i--;
		}
	}
	executable->argc = count;


	// Parse arguments
	executable->argv = malloc(count * sizeof(char*));
	count = 0;

	for (i = 0; string[i] != '\0'; i++) {
		// Skip space
		while (string[i] == ' ') {
			i++;
		}

		int start = i;
		if (string[i] == '\"') {
			i++;
			start = i;
			while (string[i] != '\0' && (string[i] != '\"' || string[i - 1] == '\\')) {
				i++;
			}
			if (string[i] != '\0' && string[i + 1] != '\"') {
				int end = i;
				int to_copy = end - start;
				executable->argv[count++] = strndup(&string[start], to_copy);

			}
		} else if (string[i] != '\0') {
			i++;
			while (string[i] != '\0' && (string[i] != ' ' || string [i - 1] == '\\')) {
				i++;
			}
			int end = i;
			int to_copy = end - start;
			executable->argv[count++] = strndup(&string[start], to_copy);
			i--;
		}
	}

	return node;
}


Node * create_pipe_from_strings (SplitResult *pieces) {
	Node *node = new_node();
	node->type = PipeNode_T;
	OperandsNode *pipe = &node->value.operands;
	pipe->count = pieces->count;
	pipe->nodes = malloc(pieces->count * sizeof(Node*));

	int i;
	for (i = 0; i < pieces->count; i++) {
		pipe->nodes[i] = create_tree_from_string(pieces->sub_strings[i]);
	}

	return node;
}

Node * create_operands_from_string (SplitResult *pieces, NodeType type) {
	Node *node = new_node();
	node->type = type;
	OperandsNode *andNode = &node->value.operands;
	andNode->count = pieces->count;
	andNode->nodes = malloc(pieces->count * sizeof(Node*));

	int i = 0;
	for (i = 0; i < andNode->count; i++) {
		andNode->nodes[i] = create_tree_from_string(pieces->sub_strings[i]);
	}

	return node;
}

Node * create_and_from_strings (SplitResult *pieces) {
	return create_operands_from_string(pieces, AndNode_T);
}


Node * create_or_from_strings (SplitResult *pieces) {
	return create_operands_from_string(pieces, OrNode_T);
}
