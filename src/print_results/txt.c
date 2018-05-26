#include <stdio.h>
#include "txt.h"


void txt_head(PrinterContext *context, Node *node) {
    fprintf(context->out, "\n\n");
}

void txt_executable_head(PrinterContext *context, Node *node) {
    fprintf(context->out, "------------------------------------\n");
    fprintf(context->out, "COMMAND\t%s\n", context->command);
    fprintf(context->out, "PATH\t%s\n", node->value.executable.path);
    fprintf(context->out, "ID #%ld.%d", context->index, context->command_subindex++);
    fprintf(context->out, "\n\n");
}


void txt_pid_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "PID: %d\n", node->pid);
}

void txt_exit_code_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Exit Code: %d\n", node->result->exit_code);
}

void txt_execution_failed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Execution OK: %d\n", !node->result->execution_failed);
}

void txt_start_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Start time: %ld\n", node->result->start_time);
}

void txt_end_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "End time: %ld\n", node->result->end_time);
}

void txt_total_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Total time: %ld\n", get_total_clock_time(node));
}

void txt_user_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "User CPU time: %ld.%06ld\n",
            node->result->user_cpu_time_used.tv_sec,
            (long) node->result->user_cpu_time_used.tv_usec);
}

void txt_system_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "System CPU time: %ld.%06ld\n",
            node->result->system_cpu_time_used.tv_sec,
            (long) node->result->user_cpu_time_used.tv_usec);
}

void txt_maximum_resident_set_size_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Maximum resident set size: %ld\n",
            node->result->maximum_resident_set_size);
}






void txt_foot (PrinterContext *context, Node *node) {

}

void txt_executable_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "------------------------------------\n");
}

