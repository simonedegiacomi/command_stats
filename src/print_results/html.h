
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


extern Printer HtmlPrinter;



#endif