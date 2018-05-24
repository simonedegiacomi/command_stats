#include <stdio.h>
#include <regex.h>
#include <memory.h>
#include "my_assert.h"
#include "../src/common/common.h"

void should_count_or_occurrences () {
	char *command 	= "true || false || true";
	char *regex 	= "\\|\\|";

	// compile
	regex_t compiled_regex;
	int res = regcomp(&compiled_regex, regex, REG_EXTENDED);
	my_assert(res == 0, "regex %s is not valid", regex);

	regmatch_t matches[1];
	int count = 0;
	char *str = command;
	while (1) {
		res = regexec(&compiled_regex, str, 1, matches, 0);

		if (res == REG_NOMATCH) {
			break;
		} else {
			count++;
			str += matches[0].rm_eo;
		}
	};
	regfree(&compiled_regex);
	my_assert(count == 2, "found %d occurrences instead of 2", count);
}

typedef struct TestEntry {
	char *regex;
	int expected_count;
} TestEntry;

void should_count_occurrences_of_similar_separators () {
	char *command = "ls | wc || true >> file.a && ls > /dev/null";

	TestEntry tests[] = {
		{ "\\|\\|", 1 }, // ||
		{ "[^|]\\|[^|]", 1 }, // |
		{ ">>", 1 },
		{ "[^>]>[^>]", 1 }
	};
	
	int tests_count = sizeof(tests) / sizeof(TestEntry);
	int i;
	for (i = 0; i < tests_count; i++) {
		TestEntry entry = tests[i];
		regex_t compiled_regex;
		int res = regcomp(&compiled_regex, entry.regex, REG_EXTENDED);
		my_assert(res == 0, "regex %s is not valid", entry.regex);

		regmatch_t matches[1];
		char *str = command;
		int count = 0;
		while (1) {
			res = regexec(&compiled_regex, str, 1, matches, 0);
			if (res == REG_NOMATCH) {
				break;
			} else {
				count++;
				str += matches[0].rm_eo;
			}
		}
		regfree(&compiled_regex);
		my_assert(count == entry.expected_count,
			"found %d occurrences instead of %d with the pattern %s",
			count, entry.expected_count, entry.regex);
	}

}


void run_experiment_regex_tests () {
	printf("[REGEX EXPERIMENT] Start tests\n");

	should_count_or_occurrences();
	should_count_occurrences_of_similar_separators();

	printf("[REGEX EXPERIMENT] All test passed\n");
}

#ifndef MAIN_TESTS
int main (int argc, char *argv[]) {
	run_experiment_regex_tests();
}
#endif



