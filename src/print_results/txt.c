#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "txt.h"


Printer TxtPrinter = {
    .head                                       = txt_head,
    .executable_head                            = txt_executable_head,
    .enter_operand_node                         = NULL,

    .executed_to_string                         = txt_executed_to_string,
    .pid_to_string                              = txt_pid_to_string,
    .exit_code_to_string                        = txt_exit_code_to_string,
    .invocation_failed_to_string                = txt_execution_failed_to_string,
    .start_time_to_string                       = txt_start_time_to_string,
    .end_time_to_string                         = txt_end_time_to_string,
    .total_time_to_string                       = txt_total_time_to_string,
    .user_cpu_time_to_string                    = txt_user_cpu_time_to_string,
    .system_cpu_time_to_string                  = txt_system_cpu_time_to_string,
    .maximum_resident_set_size_to_string        = txt_maximum_resident_set_size_to_string,

    .foot                                       = txt_foot,
    .exit_operand_node                          = NULL,
    .executable_foot                            = txt_executable_foot,
};



void txt_head(PrinterContext *context, Node *node) {
    fprintf(context->out, "Execution of '%s'\n", context->command);
    fprintf(context->out, "------------------------------------\n");
}

void txt_executable_head(PrinterContext *context, Node *node) {
    fprintf(context->out, "------------------------------------\n");
    fprintf(context->out, "COMMAND:\t\t%s\n", context->command);
    fprintf(context->out, "PATH:\t\t\t%s\n", node->value.executable.path);
    fprintf(context->out, "ID:\t\t\t#%ld.%d", context->index, context->command_subindex++);
    fprintf(context->out, "\n\n");
}


void txt_executed_to_string(PrinterContext *context, Node *node) {

}

void txt_pid_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "PID:\t\t\t%d\n", node->pid);
}

void txt_exit_code_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Exit Code:\t\t%d\n", WEXITSTATUS(node->result->exit_code));
}

void txt_execution_failed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Execution OK:\t\t%s\n", node->result->invocation_failed ? "false" : "true");
}

void txt_start_time_to_string(PrinterContext *context, Node *node) {
    time_t start = node->result->start_time.tv_sec;
    fprintf(context->out, "Start time:\t\t%s", ctime(&start));
}

void txt_end_time_to_string(PrinterContext *context, Node *node) {
    time_t end = node->result->end_time.tv_sec;
    fprintf(context->out, "End time:\t\t%s", ctime(&end));
}

void txt_total_time_to_string(PrinterContext *context, Node *node) {
    struct timespec total_time = get_total_clock_time(node);

    fprintf(context->out, "Total time:\t\t");
    print_timespec(total_time, context->out);
    fprintf(context->out, "\n");
}


void txt_user_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "User CPU time:\t\t");
    print_timeval(node->result->user_cpu_time_used, context->out);
    fprintf(context->out, "\n");
}

void txt_system_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "System CPU time:\t");

    print_timeval(node->result->system_cpu_time_used, context->out);
    fprintf(context->out, "\n");
}

void txt_maximum_resident_set_size_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "Max resident set:\t%ld\n",
            node->result->maximum_resident_set_size);
}



void txt_executable_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "------------------------------------\n");
}

void txt_foot (PrinterContext *context, Node *node) {
    fprintf(context->out, "\n\n\n");
}
