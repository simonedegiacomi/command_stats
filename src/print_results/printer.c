#include <unistd.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "printer.h"
#include "../common/my_regex.h"


Attribute parse_attribute (const char *attribute_string) {
    int i;
    for (i = 0; i < keywords_count; i++) {
        if (strcmp(attribute_string, keywords[i].keyword) == 0) {
            return keywords[i].value;
        }
    }
    return INVALID_ATTRIBUTE;
}


const char *get_attribute_name(Attribute attribute) {
    int i;
    for (i = 0; i < keywords_count; i++) {
        if (keywords[i].value == attribute) {
            return keywords[i].keyword;
        }
    }
    return NULL;
}

void parse_attributes(const char *attributes_string, PrinterContext *context) {
    const regex_t * compiled = compile_regex(",");
    SplitResult *split = split_string(attributes_string, compiled);

    Attribute *attributes = malloc(split->count * sizeof(Attribute));
    int i, j = 0;
    for (i = 0; i < split->count; i++) {
        Attribute attribute = parse_attribute(split->sub_strings[i]);
        if (attribute != INVALID_ATTRIBUTE) {
            attributes[j++] = attribute;
        }
    }

    context->attributes = attributes;
    context->attributes_count = j;

    regfree((regex_t *) compiled);
}

