#ifndef PRINT_RESULTS_COMMON_H
#define PRINT_RESULTS_COMMON_H

#include <stdio.h>
#include "../structs/node.h"

typedef enum Attribute {
    INVALID_ATTRIBUTE,
    PID,
    EXIT_CODE,
    EXECUTION_FAILED,
    START_TIME,
    END_TIME,
    TOTAL_TIME,
    USER_CPU_TIME,
    SYSTEM_CPU_TIME,
    MAXIMUM_RESIDENT_SEGMENT_SIZE, // TODO: rename
} Attribute;

typedef struct AttributeKeyword {
    const char  *keyword;
    Attribute   value;
} AttributeKeyword;

static AttributeKeyword keywords[] = {
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
static int keywords_count = sizeof(keywords) / sizeof(AttributeKeyword);



typedef struct PrinterContext {
    const char      *command;
    long            index;
    int             command_subindex;
    Attribute       *attributes;
    int             attributes_count;
    FILE            *out;
} PrinterContext;


typedef void (*ToString) (PrinterContext *context, Node *node);

typedef struct Printer {
    ToString head;
    ToString executable_head;
    ToString enter_operand_node;

    ToString executed_to_string;
    ToString invocation_failed_to_string;
    ToString pid_to_string;
    ToString exit_code_to_string;
    ToString start_time_to_string;
    ToString end_time_to_string;
    ToString total_time_to_string;
    ToString user_cpu_time_to_string;
    ToString system_cpu_time_to_string;
    ToString maximum_resident_set_size_to_string;

    ToString executable_foot;
    ToString exit_operand_node;
    ToString foot;

} Printer;



Attribute       parse_attribute         (const char *attribute_string);
void            parse_attributes        (const char *attributes_string, PrinterContext *context);
const char *    get_attribute_name      (Attribute attribute);


#endif
