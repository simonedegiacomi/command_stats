#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "wire.h"

void wire_r (Node *tree, int stdin, int stdout);

void wire (Node *tree) {
	wire_r(tree, STDIN_FILENO, STDOUT_FILENO);
}


void wire_r (Node *tree, int in, int out) {
	
}



