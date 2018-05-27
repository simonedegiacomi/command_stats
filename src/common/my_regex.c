#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_regex.h"

const char *create_string_from_match(const char *str, const regmatch_t *match);


const regex_t * compile_regex (const char *regex) {
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

const char * create_string_from_match(const char *str, const regmatch_t *match) {
    const char *start   = str + match->rm_so;
    const char *end     = str + match->rm_eo;
    size_t to_copy      = end - start;
    return strndup(start, to_copy);
}

int count_occurrences_of_regex (const char *string, const regex_t *delimiter) {
    regmatch_t match;
    int count = 0;
    while (string[0] != '\0') {
        int res = regexec(delimiter, string, 1, &match, 0);

        if (res == REG_NOMATCH) {
            break;
        } else {
            count++;
            string += match.rm_eo;

            if (match.rm_eo - match.rm_so <= 0) {
                string++;
            }
        }
    };

    return count;
}

SplitResult * split_string(const char *string, const regex_t *delimiter) {
    return split_obfuscated_string(string, delimiter, 0, string);
}

SplitResult *split_obfuscated_string(const char *obfuscated_string, const regex_t *delimiter, int group, const char *string) {
    int operators_count = count_occurrences_of_regex(obfuscated_string, delimiter);

    int pieces = operators_count + 1;
    SplitResult *split = malloc(sizeof(SplitResult));
    split->count = pieces;
    split->sub_strings = malloc(pieces * sizeof(char *));


    int i;
    const char *obs_str = obfuscated_string;
    const char *str = string;

    size_t matches_size = (size_t) group + 1;
    regmatch_t *matches = malloc(matches_size * sizeof(regmatch_t));


    for (i = 0; i < pieces; i++) {
        int res = regexec(delimiter, obs_str, matches_size, matches, 0);

        regmatch_t *match = &matches[group];

        const char *start = str;
        size_t to_copy;

        if (res == REG_NOMATCH) {
            to_copy = strlen(obs_str);
            printf("OBS: %s tc %d\n", obs_str, to_copy);
        } else {
            to_copy = (size_t) match->rm_so;
            printf("SO: %d\tEO: %d\n", match->rm_so, match->rm_eo);
        }

        split->sub_strings[i] = strndup(start, to_copy);

        printf("1 OBS: %s %d\n", obs_str, match->rm_eo);
        obs_str += match->rm_eo;
        str += match->rm_eo;
    }

    return split;
}


void destroy_split (SplitResult *to_destroy) {
    int i;
    for (i = 0; i < to_destroy->count; i++) {
        free((void *) to_destroy->sub_strings[i]);
    }

    free(to_destroy->sub_strings);
    free(to_destroy);
}

