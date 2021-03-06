#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <sys/wait.h>
#include "csv.h"
#include "../structs/node.h"

#define CTIME_BUFFER 26


Printer CsvPrinter = {
    .head                                       = csv_head,
    .executable_head                            = csv_executable_head,
    .enter_operand_node                         = NULL,

    .executed_to_string                         = csv_executed_to_string,
    .pid_to_string                              = csv_pid_to_string,
    .exit_code_to_string                        = csv_exit_code_to_string,
    .invocation_failed_to_string                = csv_execution_failed_to_string,
    .start_time_to_string                       = csv_start_time_to_string,
    .end_time_to_string                         = csv_end_time_to_string,
    .total_time_to_string                       = csv_total_time_to_string,
    .user_cpu_time_to_string                    = csv_user_cpu_time_to_string,
    .system_cpu_time_to_string                  = csv_system_cpu_time_to_string,
    .maximum_resident_set_size_to_string        = csv_maximum_resident_set_size_to_string,

    .exit_operand_node                          = NULL,
    .foot                                       = csv_foot,
    .executable_foot                            = csv_executable_foot,
};



static BOOL previous_value = FALSE;

void my_ctime (time_t start, char *buffer);


void separate_argument(PrinterContext *context){
    if (previous_value) {
        fprintf(context->out, ",");
    } else {
        previous_value = TRUE;
    }
}

void print_csv_value (PrinterContext *context, const char *format, ...) {
    separate_argument(context);

    va_list ap;
    va_start(ap, format);
    vfprintf(context->out, format, ap);
    va_end(ap);
}

void csv_head(PrinterContext *context, Node *node) {
    previous_value = FALSE;

    print_csv_value(context, "executable_name");
    int i;
    for (i = 0; i < context->attributes_count; i++) {
        print_csv_value(context, "%s", get_attribute_name(context->attributes[i]));
    }

    fprintf(context->out, "\n");
    previous_value = FALSE;
}


void csv_executable_head(PrinterContext *context, Node *node) {
    print_csv_value(context, "%s", node->value.executable.path);
}


void csv_executed_to_string(PrinterContext *context, Node *node) {

}

void csv_pid_to_string(PrinterContext *context, Node *node) {
    print_csv_value(context, "%d", node->pid);
}

void csv_exit_code_to_string(PrinterContext *context, Node *node) {
    print_csv_value(context, "%d", WEXITSTATUS(node->result->exit_code));
}

void csv_execution_failed_to_string(PrinterContext *context, Node *node) {
    print_csv_value(context, "%s", node->result->invocation_failed ? "true" : "false");
}

void csv_start_time_to_string(PrinterContext *context, Node *node) {
    time_t start = node->result->start_time.tv_sec;
    char buffer[CTIME_BUFFER];
    my_ctime(start, buffer);
    print_csv_value(context, "%s", buffer);
}

void my_ctime (time_t start, char *buffer) {
    ctime_r(&start, buffer);
    buffer[strlen(buffer) - 1] = '\0';
}


void csv_end_time_to_string(PrinterContext *context, Node *node) {
    time_t end = node->result->end_time.tv_sec;
    char buffer[CTIME_BUFFER];
    my_ctime(end, buffer);
    print_csv_value(context, "%s", buffer);
}

void csv_total_time_to_string(PrinterContext *context, Node *node) {
    separate_argument(context);
    struct timespec total_time = get_total_clock_time(node);
    print_timespec(total_time, context->out);
}

void csv_user_cpu_time_to_string(PrinterContext *context, Node *node) {
    separate_argument(context);
    print_timeval(node->result->user_cpu_time_used, context->out);
}

void csv_system_cpu_time_to_string(PrinterContext *context, Node *node) {
    separate_argument(context);
    print_timeval(node->result->system_cpu_time_used, context->out);
}

void csv_maximum_resident_set_size_to_string(PrinterContext *context, Node *node) {
    print_csv_value(context, "%ld",
            node->result->maximum_resident_set_size);
}


void csv_foot (PrinterContext *context, Node *node) {

}

void csv_executable_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "\n");
    previous_value = FALSE;
}

