#ifndef MY_REGEX_H
#define MY_REGEX_H

#include <regex.h>

typedef struct SplitResult {
    int count;
    const char **sub_strings;
} SplitResult;

regex_t *       compile_regex               (const char *regex);
const char *    create_string_from_match    (const char *str, const regmatch_t *match);
int             count_occurrences_of_regex  (const char *string, const regex_t *delimiter);
SplitResult *   split_string                (const char *string, const regex_t *delimiter);
SplitResult *   split_obfuscated_string     (const char *obfuscated_string, const regex_t *delimiter, int group, const char *string);
void            destroy_split               (SplitResult *to_destroy);

#endif