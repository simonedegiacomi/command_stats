#include <stdio.h>
#include "to_string.h"



void pid_to_string(PrinterContext *context, Node *node) {
    if (context->format == TXT) {
        fprintf(context->out, "PID: %d\n", node->pid);
    } else if (context->format == CSV) {

    }
}

void exit_code_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Exit Code: %d\n", node->result->exit_code);
}

void execution_failed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Execution OK: %d\n", !node->result->execution_failed);
}

void start_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Start time: %ld\n", node->result->start_time);
}

void end_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "End time: %ld\n", node->result->end_time);
}

void total_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Total time: %ld\n",
            get_total_time(node->result->start_time, node->result->end_time));
}

void user_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "User CPU time: %ld.%06ld\n",
            node->result->user_cpu_time_used.tv_sec,
            (long) node->result->user_cpu_time_used.tv_usec);
}

void system_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "System CPU time: %ld.%06ld\n",
            node->result->system_cpu_time_used.tv_sec,
            (long) node->result->user_cpu_time_used.tv_usec);
}

void maximum_resident_set_size_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Maximum resident set size: %ld\n",
            node->result->maximum_resident_set_size);
}


