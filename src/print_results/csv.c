#include <stdio.h>
#include "csv.h"


void csv_head(PrinterContext *context, Node *node) {

}

void csv_executable_head(PrinterContext *context, Node *node) {
    int i;
    for (i = 0; i < context->attributes_count; i++) {
        fprintf(context->out, "%s,", get_attribute_name(context->attributes[i]));
    }

    fprintf(context->out, "\n");
}


void csv_pid_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%d,", node->pid);
}

void csv_exit_code_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%d,", node->result->exit_code);
}

void csv_execution_failed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%s,", node->result->execution_failed ? "true" : "false");
}

void csv_start_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%ld,", node->result->start_time);
}

void csv_end_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%ld,", node->result->end_time);
}

void csv_total_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%ld,", get_total_clock_time(node));
}

void csv_user_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%ld .%06ld,",
            node->result->user_cpu_time_used.tv_sec,
            (long) node->result->user_cpu_time_used.tv_usec);
}

void csv_system_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%ld.%06ld,",
            node->result->system_cpu_time_used.tv_sec,
            (long) node->result->user_cpu_time_used.tv_usec);
}

void csv_maximum_resident_set_size_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "%ld,",
            node->result->maximum_resident_set_size);
}


void csv_foot (PrinterContext *context, Node *node) {

}

void csv_executable_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "\n");
}

