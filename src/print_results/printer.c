#include <unistd.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "printer.h"
#include "../common/my_regex.h"

// Association between keyword and enum
AttributeKeyword keywords[] = {
    {
        .keyword = "pid",
        .value = PID
    },
    {
        .keyword = "exit_code",
        .value = EXIT_CODE
    },
    {
        .keyword = "invocation_failed",
        .value = EXECUTION_FAILED
    },
    {
        .keyword = "start_time",
        .value = START_TIME
    },
    {
        .keyword = "end_time",
        .value = END_TIME
    },
    {
        .keyword = "total_time",
        .value = TOTAL_TIME
    },
    {
        .keyword = "user_cpu_time",
        .value = USER_CPU_TIME
    },
    {
        .keyword = "system_cpu_time",
        .value = SYSTEM_CPU_TIME
    },
    {
        .keyword = "maximum_resident_set_size",
        .value = MAXIMUM_RESIDENT_SEGMENT_SIZE
    }
};
int keywords_count = sizeof(keywords) / sizeof(AttributeKeyword);

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

