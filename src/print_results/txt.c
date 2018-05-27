#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "txt.h"


void txt_head(PrinterContext *context, Node *node) {
    fprintf(context->out, "Execution of '%s'\n", context->command);
    fprintf(context->out, "------------------------------------\n");
}

void txt_executable_head(PrinterContext *context, Node *node) {
    fprintf(context->out, "------------------------------------\n");
    fprintf(context->out, "COMMAND:\t%s\n", context->command);
    fprintf(context->out, "PATH:\t\t%s\n", node->value.executable.path);
    fprintf(context->out, "ID:\t\t#%ld.%d", context->index, context->command_subindex++);
    fprintf(context->out, "\n\n");
}


void txt_executed_to_string(PrinterContext *context, Node *node) {

}

void txt_pid_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "PID:\t%d\n", node->pid);
}

void txt_exit_code_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Exit Code:\t%d\n", WEXITSTATUS(node->result->exit_code));
}

void txt_execution_failed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Execution OK:\t%s\n", node->result->invocation_failed ? "true" : "false");
}

void txt_start_time_to_string(PrinterContext *context, Node *node) {
    time_t start = node->result->start_time.tv_sec;
    fprintf(context->out, "Start time:\t%s", ctime(&start));
}

void txt_end_time_to_string(PrinterContext *context, Node *node) {
    time_t end = node->result->end_time.tv_sec;
    fprintf(context->out, "End time:\t%s", ctime(&end));
}

void txt_total_time_to_string(PrinterContext *context, Node *node) {
    struct timespec total_time = get_total_clock_time(node);

    fprintf(context->out, "Total time:\t");
    print_time(total_time, context->out);
    fprintf(context->out, "\n");
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





void txt_executable_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "------------------------------------\n");
}

void txt_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "\n\n\n");
}
