#define MAIN_TESTS
#include "experiment_regex.c"
#include "check_parse.c"
#include "check_wire.c"


void run_all_tests () {
	run_experiment_regex_tests();
	run_parser_test();
	run_wire_tests();
}

int main (int argc, char *argv[]) {
	run_all_tests();
	return 0;
}
