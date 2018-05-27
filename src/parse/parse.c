#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "parse.h"
#include "../common/my_regex.h"
#include "builtin.h"

/** Private functions and structures declaration */
void initialize_regexes();
void finalize_regexes();

SplitResult * create_split_for_operator(const char *string, const regex_t *separator);

// contains the substring of a command and, if any, the file names to which redirect the input/output
typedef struct RedirectSplit {
    const char  *command;
    BOOL        has_redirects;

    Stream      *in, *out;
} RedirectSplit;


RedirectSplit * create_split_command_from_redirect(const char *str);
void redirect_streams_of_node_to_files(Node *parsed, RedirectSplit *string_and_redirects);

const char * remove_brackets_if_alone(const char *str);
Node * create_node_from_string(const char *raw_string);

// constructors of the nodes from the split strings
Node * create_pipe_from_strings			(SplitResult *pieces);
Node * create_and_from_strings			(SplitResult *pieces);
Node * create_or_from_strings			(SplitResult *pieces);
Node * create_semicolon_from_strings	(SplitResult *pieces);
Node * create_executable_from_string	(SplitResult *pieces);

/** End of private functions and structures declaration */


// Entry for the array that describes the priority of the operands.
typedef struct PriorityMapEntry {

    // Regex that matches the separator of the operand
    char 		    *separator_regex;

    regex_t         *compiled_separator;

    // True if the handler can be called even if the SplitResult contains only a string
    BOOL            single_split;

    // Function to construct a node from the split string
    Node *(*handler) (SplitResult*);
} PriorityMapEntry;

static PriorityMapEntry priority_map[] = {
        {
                // And
                .separator_regex 	= "(&&)",
                .handler 			= create_and_from_strings,
                .single_split       = FALSE
        },
        {
                // Or operator
                .separator_regex	= "(\\|\\|)",
                .handler 			= create_or_from_strings,
                .single_split       = FALSE
        },
        {
                // Pipe operator
                .separator_regex	= "[^|](\\|)[^|]",
                .handler 			= create_pipe_from_strings,
                .single_split       = FALSE
        },
        {
                // Semicolon
                .separator_regex    = "(;)",
                .handler            = create_semicolon_from_strings,
                .single_split       = FALSE
        },
        {
                // Executable
                .separator_regex    = "($)",
                .handler            = create_executable_from_string,
                .single_split       = TRUE
        }
};
static const int operators_count = sizeof(priority_map) / sizeof(PriorityMapEntry);


static BOOL parser_initialized = FALSE;

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
}

void finalize_parser () {
    if (!parser_initialized) {
        return;
    }
    finalize_regexes();
    parser_initialized = FALSE;
}

void finalize_regexes() {
    // priority map regexes
    int i;
    for (i = 0; i < operators_count; i++) {
        regfree(priority_map[i].compiled_separator);
    }
}


Node * create_tree_from_string (const char *raw_string) {
    Node *tree = create_node_from_string(raw_string);
    apply_builtin(tree);
    return tree;
}

Node * create_node_from_string(const char *raw_string) {
    initialize_parser();


    RedirectSplit   *string_and_redirects   = create_split_command_from_redirect(raw_string);
    const char      *string                 = remove_brackets_if_alone(string_and_redirects->command);

    int i;
    Node *parsed = NULL;
    for (i = 0; i < operators_count && parsed == NULL; i++) {

        PriorityMapEntry *entry = &priority_map[i];
        SplitResult *pieces     = create_split_for_operator(string, entry->compiled_separator);

        if (entry->single_split || pieces->count > 1) {
            parsed = entry->handler(pieces);
        }

        destroy_split(pieces);
    }

    if (parsed == NULL) {
        program_fail("[PARSER] Can't parse command.\n");
    }

    if (string_and_redirects->has_redirects) {
        redirect_streams_of_node_to_files(parsed, string_and_redirects);
    }

    return parsed;
}



/**
 * Return a new string (that can be freed using free) without the brackets if
 * they are only around the start and the end of the string.
 * Examples:
 * - (true && false) => true && false
 * - ((true) && false) => (true) && false
 * - (true) && (false) => (true) && (false)
 */
const char * remove_brackets_if_alone(const char *str) {
    char *const MATCH_STRING_INSIDE_BRACKETS_IF_ALONE = "^[ ]*\\((.*)\\)[ ]*$";
    regex_t *string_inside_brackets_regex = compile_regex(MATCH_STRING_INSIDE_BRACKETS_IF_ALONE);

    // Remove brackets using regex
    regmatch_t matches[2];
    int res = regexec(string_inside_brackets_regex, str, 2, matches, 0);
    regmatch_t *match = &matches[1];

    regfree(string_inside_brackets_regex);

    if (res != REG_NOMATCH) {
        const char *without_brackets = create_string_from_match(str, match);

        // Check if the brakets were only at the start and end of the string,
        // checking if now some brackets group are broken
        int i;
        int open = 0, close = 0;
        BOOL broken = FALSE;
        for (i = 0; i < strlen(without_brackets) && !broken; i++) {
            if (without_brackets[i] == '\\') { // skip backslash
                i++; // skip also next character
            } else {

                if (without_brackets[i] == '(') {
                    open++;
                } else if (without_brackets[i] == ')') {
                    close++;
                    if (close > open) {
                        broken = TRUE;
                    }
                }
            }
        }

        if (!broken) {
            return without_brackets;
        } else {
            free((void *) without_brackets);
        }
    }

    regfree((regex_t *) string_inside_brackets_regex);

    return strdup(str);

    /*
    //char *const MATCH_STRING_INSIDE_BRACKETS_IF_ALONE = "^[ ]*\\((.*)\\)[ ]*$";
    char *const MATCH_STRING_INSIDE_BRACKETS_IF_ALONE = "^[ ]*\\(((\\\\\\(|[^\\(\\)]|\\\\\\))*)\\)[ ]*$";
    const int COMMAND_GROUP = 1;

    const regex_t *regex = compile_regex(MATCH_STRING_INSIDE_BRACKETS_IF_ALONE);

    regmatch_t matches[3];
    int res = regexec(regex, str, 3, matches, 0);
    regmatch_t *match = &matches[COMMAND_GROUP];

    if (res != REG_NOMATCH) {
        return create_string_from_match(str, match);
    }

    regfree((regex_t *) regex);

    return strdup(str);*/

    /*
    //char *const MATCH_STRING_INSIDE_BRACKETS_IF_ALONE = "^[ ]*\\((.*)\\)[ ]*$";
    char *const MATCH_STRING_INSIDE_BRACKETS_IF_ALONE = "^[ ]*\\(((\\\\\\(|[^\\(\\)]|\\\\\\))*)\\)[ ]*$";
    const int COMMAND_GROUP = 1;

    const regex_t *regex = compile_regex(MATCH_STRING_INSIDE_BRACKETS_IF_ALONE);

    regmatch_t matches[3];
    int res = regexec(regex, str, 3, matches, 0);
    regmatch_t *match = &matches[COMMAND_GROUP];

    if (res != REG_NOMATCH) {
        return create_string_from_match(str, match);
    }

    regfree((regex_t *) regex);

    return strdup(str);*/
}




RedirectSplit * create_redirect_with_command_from_string (const char *str) {
    RedirectSplit *split1 = malloc(sizeof(RedirectSplit));;
    split1->in = split1->out = NULL;
    RedirectSplit *split = split1;

    char *const MATCH_REDIRECT_AND_FILE_NAME_IF_LAST = "((>>|>|<) *([a-zA-Z\\.]*))+$";
    regex_t *regex = compile_regex(MATCH_REDIRECT_AND_FILE_NAME_IF_LAST); // explain groups and SPACE*

    regmatch_t match;
    int res = regexec(regex, str, 1, &match, 0);
    if (res != REG_NOMATCH) {
        const char *start = str;
        const char *end = str + match.rm_so;
        size_t to_copy = end - start;

        split->command = strndup(start, to_copy);

        split->has_redirects = TRUE;
    } else {
        split->has_redirects = FALSE;
        split->command = strdup(str);
    }

    regfree(regex);

    return split;
}

void fill_redirect_with_redirects_and_file_names(RedirectSplit *split, const char *string) {
    char *const MATCH_REDIRECT_AND_FILE_NAME_IF_LAST = "((>>|>|<) *([a-zA-Z\\.]*))+$";
    regex_t *regex = compile_regex(MATCH_REDIRECT_AND_FILE_NAME_IF_LAST); // explain groups and SPACE*

    regmatch_t matches[4];
    const char *str;
    for (str = string; regexec(regex, str, 4, matches, 0) != REG_NOMATCH; str += matches[3].rm_eo) {
        const char *redirect_type   = create_string_from_match(str, &matches[2]);
        const char *file_name       = create_string_from_match(str, &matches[3]);

        Stream *stream;
        if (redirect_type[0] == '>') {
            if (split->out == NULL) {
                split->out = malloc(sizeof(Stream));
            }
            stream = split->out;

            stream->options.file.open_flag = O_CREAT | O_WRONLY;
            if (strlen(redirect_type) == 1) { // >
                stream->options.file.open_flag |= O_TRUNC;
            } else { // >>
                stream->options.file.open_flag |= O_APPEND;
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

    regfree(regex);
}


RedirectSplit * create_split_command_from_redirect(const char *str) {
    RedirectSplit *split = create_redirect_with_command_from_string(str);

    if (split->has_redirects) {
        fill_redirect_with_redirects_and_file_names(split, str);
    }

    return split;
}

void redirect_streams_of_node_to_files(Node *parsed, RedirectSplit *string_and_redirects) {
    if (string_and_redirects->in != NULL) {
        parsed->std_in = string_and_redirects->in;
    }
    if (string_and_redirects->out != NULL) {
        parsed->std_out = string_and_redirects->out;
    }
}


const char * obfuscate_brackets_in_string(const char *str) {
    size_t len = strlen(str);
    char *obs = strdup(str);

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

    return obs;
}

SplitResult * create_split_for_operator(const char *string, const regex_t *separator) {
    const char *obfuscated_string = obfuscate_brackets_in_string(string);
    SplitResult * res = split_obfuscated_string(obfuscated_string, separator, 1, string);
    free((void *) obfuscated_string);
    return res;
}


char * clear_argument (const char *to_clear) {
    if (strchr(to_clear, '\\') == NULL) {
        return strdup(to_clear);
    }

    const char *CLEAR_ARGUMENTS = "([^\\\\]+)";

    regex_t *regex = compile_regex(CLEAR_ARGUMENTS);
    regmatch_t match;

    char *res = malloc(strlen(to_clear) * sizeof(char));
    res[0] = '\0';
    while (regexec(regex, to_clear, 1, &match, 0) != REG_NOMATCH) {
        const char *start = to_clear + match.rm_so;
        const char *end = to_clear + match.rm_eo;
        strncat(res, start, end-start);
        to_clear += match.rm_eo;

    }

    regfree(regex);

    return res;
}


Node * create_executable_from_string (SplitResult *pieces) {
    const char *MATCH_ARGUMENTS = "( *\"(([^\"\\\\]|\\\\.)+)\" *)|( *([^\"][^ ]*) *)";
    regex_t *regex = compile_regex(MATCH_ARGUMENTS);

    const char *string = pieces->sub_strings[0];
    int arguments_count = count_occurrences_of_regex(string, regex);
    if (arguments_count <= 0) {
        program_fail("[PARSER] Can't parse command.\n");
    }

    char **argv = malloc((arguments_count + 1) * sizeof(char*)); // The last string of argv must be a null pointer

    int i;
    const char *last_arg_end = NULL;
    BOOL last_arg_was_quoted = FALSE;
    int joined_arguments = 0;
    for (i = 0; i < arguments_count; i++) {
        regmatch_t matches[6];


        int res = regexec(regex, string, 6, matches, 0);
        if (res == REG_NOMATCH) {
            fprintf(stderr, "[PARSER] ATTENZIONE, REGEXP VUOTA\n");
            break;
        }


        // select group
        regmatch_t *match_to_use = &matches[5]; // 5 -> not quoted
        BOOL arg_is_quoted = FALSE;
        if (match_to_use->rm_so == match_to_use->rm_eo) {
            arg_is_quoted = TRUE;
            match_to_use = &matches[2]; // 2 -> quoted
        }


        const char *start = string + match_to_use->rm_so;
        const char *end = string + match_to_use->rm_eo;

        char *temp = strndup(start, (end - start));
        if (arg_is_quoted && last_arg_was_quoted && (start - 2) == last_arg_end) {
            i--;

            argv[i] = strcat(argv[i], clear_argument(temp));
            joined_arguments++;
        } else {
            argv[i] = clear_argument(temp);
        }
        free(temp);

        last_arg_end = end;
        last_arg_was_quoted = arg_is_quoted;

        string = string + matches[0].rm_eo;
    }

    argv[arguments_count] = NULL;

    Node *node = create_executable_node(argv[0]);
    node->value.executable.argv = argv;
    node->value.executable.argc = arguments_count - joined_arguments;

    regfree(regex);

    return node;
}


Node * create_pipe_from_strings (SplitResult *pieces) {
    Node *node = create_node();
    node->type = PipeNode_T;
    OperandsNode *pipe = &node->value.operands;
    pipe->appender = NULL;
    pipe->count = pieces->count;
    pipe->nodes = malloc(pieces->count * sizeof(Node*));

    int i;
    for (i = 0; i < pieces->count; i++) {
        pipe->nodes[i] = create_node_from_string(pieces->sub_strings[i]);
    }
    return node;
}

Node * create_operands_from_string (SplitResult *pieces, NodeType type) {
    Node *node = create_node();
    node->type = type;
    OperandsNode *andNode = &node->value.operands;
    andNode->appender = NULL;
    andNode->count = pieces->count;
    andNode->nodes = malloc(pieces->count * sizeof(Node*));

    int i = 0;
    for (i = 0; i < andNode->count; i++) {
        andNode->nodes[i] = create_node_from_string(pieces->sub_strings[i]);
    }

    return node;
}

Node * create_and_from_strings (SplitResult *pieces) {
    return create_operands_from_string(pieces, AndNode_T);
}


Node * create_or_from_strings (SplitResult *pieces) {
    return create_operands_from_string(pieces, OrNode_T);
}


Node *create_semicolon_from_strings(SplitResult *pieces) {
    return create_operands_from_string(pieces, SemicolonNode_T);
}
