
#ifndef HTML_PRINTER_H
#define HTML_PRINTER_H


#include "printer.h"

void html_head                                   (PrinterContext *context, Node* node);
void html_executable_head                        (PrinterContext *context, Node* node);
void html_enter_operand_node                     (PrinterContext *context, Node* node);

void html_executed_to_string                     (PrinterContext *context, Node *node);
void html_invocation_failed_to_string            (PrinterContext *context, Node *node);
void html_pid_to_string                          (PrinterContext *context, Node* node);
void html_exit_code_to_string                    (PrinterContext *context, Node *node);
void html_start_time_to_string                   (PrinterContext *context, Node *node);
void html_end_time_to_string                     (PrinterContext *context, Node *node);
void html_total_time_to_string                   (PrinterContext *context, Node *node);
void html_user_cpu_time_to_string                (PrinterContext *context, Node *node);
void html_system_cpu_time_to_string              (PrinterContext *context, Node *node);
void html_maximum_resident_set_size_to_string    (PrinterContext *context, Node *node);

void html_executable_foot                        (PrinterContext *context, Node* node);
void html_exit_operand_node                     (PrinterContext *context, Node* node);
void html_foot                                   (PrinterContext *context, Node* node);


static Printer HtmlPrinter = {
    .head                                       = html_head,
    .enter_operand_node                         = html_enter_operand_node,
    .executable_head                            = html_executable_head,

    .executed_to_string                         = html_executed_to_string,
    .invocation_failed_to_string                = html_invocation_failed_to_string,
    .pid_to_string                              = html_pid_to_string,
    .exit_code_to_string                        = html_exit_code_to_string,
    .start_time_to_string                       = html_start_time_to_string,
    .end_time_to_string                         = html_end_time_to_string,
    .total_time_to_string                       = html_total_time_to_string,
    .user_cpu_time_to_string                    = html_user_cpu_time_to_string,
    .system_cpu_time_to_string                  = html_system_cpu_time_to_string,
    .maximum_resident_set_size_to_string        = html_maximum_resident_set_size_to_string,

    .executable_foot                            = html_executable_foot,
    .exit_operand_node                          = html_exit_operand_node,
    .foot                                       = html_foot,
};




#endif