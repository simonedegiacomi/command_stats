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


    ToString pid_to_string;
    ToString exit_code_to_string;
    ToString execution_failed_to_string;
    ToString start_time_to_string;
    ToString end_time_to_string;
    ToString total_time_to_string;
    ToString user_cpu_time_to_string;
    ToString system_cpu_time_to_string;
    ToString maximum_resident_set_size_to_string;

    ToString foot;
    ToString executable_foot;
} Printer;



#endif
