#ifndef PARSE_H
#define PARSE_H



#include "structs.h"

void initialize_parser();

Node * create_tree_from_string (const char *raw_string);

// Frees spaces used by the parser. After this call you can still use the parser, which will be re-initialized
void finalize_parser();

#endif
