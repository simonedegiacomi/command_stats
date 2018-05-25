#ifndef PRINT_RESULTS_COMMON_H
#define PRINT_RESULTS_COMMON_H

#include <stdio.h>


typedef enum FileFormat {
    TXT,
    CSV
} FileFormat;

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
    FileFormat      format;
} PrinterContext;



FileFormat format_from_string (const char* format_string);


#endif
