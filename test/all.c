#define MAIN_TESTS
#include "experiment_regex.c"
#include "check_parse.c"
#include "check_builtin.c"


void run_all_tests () {
	//run_experiment_regex_tests();
	run_parser_test();
	//run_builtin_tests();
}

int main (int argc, char *argv[]) {
	run_all_tests();
	return 0;
}
